#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
// #include "cJSON.h"
#include "protocol_examples_common.h"
#include "rest_server.h"
#include "keep_alive.h"

static const char *TAG = "esp-rest";
static const size_t max_clients = 6;
static httpd_handle_t server_ptr;

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
    uint32_t data;
};

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t ws_open_fd(httpd_handle_t hd, int sockfd)
{
    ESP_LOGI(TAG, "New ws client opened %d", sockfd);
    ws_keep_alive_t h = httpd_get_global_user_ctx(hd);
    return ws_keep_alive_add_client(h, sockfd);
}

void ws_close_fd(httpd_handle_t hd, int sockfd)
{
    ESP_LOGI(TAG, "WS client disconnected %d", sockfd);
    ws_keep_alive_t h = httpd_get_global_user_ctx(hd);
    ws_keep_alive_remove_client(h, sockfd);
}

static esp_err_t ws_handler(httpd_req_t *req) {
    uint8_t buf[128] = {0};
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;

    /* Receive full message first */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }

    /* Return message should be text */
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
        ESP_LOGI(TAG, "ws_handler: Received packet with message: %s", ws_pkt.payload);
        ESP_LOGI(TAG, "ws_handler: httpd_handle_t=%p, sockfd=%d, client_info:%d", req->handle,
                 httpd_req_to_sockfd(req), httpd_ws_get_fd_info(req->handle, httpd_req_to_sockfd(req)));
        return ret;
    }
    else if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
        ESP_LOGD(TAG, "Received PONG message");
        return ws_keep_alive_client_is_active(httpd_get_global_user_ctx(req->handle),
                httpd_req_to_sockfd(req));
    }
    return ESP_OK;
}

static void send_data(void *arg) {
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    char data_buf[33] = {0};
    itoa(resp_arg->data, data_buf, 16);
    // uint32_t data = resp_arg->data;

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *) data_buf;
    ws_pkt.len = 4;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

static void send_ping(void *arg) {
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;

    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = NULL;
    ws_pkt.len = 0;
    ws_pkt.type = HTTPD_WS_TYPE_PING;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

/* Send asyncronous message to all clients */
esp_err_t ws_server_notify_all(uint32_t data) {
    if (server_ptr == NULL) {
        ESP_LOGE(TAG, "ws server not setup");
        return ESP_FAIL;
    }

    size_t clients = max_clients;
    int client_fds[max_clients];

    ws_keep_alive_t h = httpd_get_global_user_ctx(server_ptr);
    esp_err_t ret = ws_get_client_list(h, client_fds);
    if (ret == ESP_OK) {
        for (size_t i = 0; i < clients; i++) {
            ESP_LOGI(TAG, "client[%d]=%d", i, client_fds[i]);
            if (client_fds[i] != -1) {
                int sd = client_fds[i];
                if (httpd_ws_get_fd_info(server_ptr, sd) == HTTPD_WS_CLIENT_WEBSOCKET) {
                    ESP_LOGI(TAG, "Active client (fd=%d) -> sending async message", sd);
                    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
                    resp_arg->hd = server_ptr;
                    resp_arg->fd = sd;
                    resp_arg->data = data;

                    if (httpd_queue_work(resp_arg->hd, send_data, resp_arg) != ESP_OK) {
                        ESP_LOGE(TAG, "httpd_queue_work failed");
                        return ESP_FAIL;
                    }
                }
            }
        }
    } else {
        ESP_LOGE(TAG, "\nhttpd_get_client_list failed %d", ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

bool client_not_alive_cb(ws_keep_alive_t h, int fd) {
    ESP_LOGE(TAG, "Client not alive, closing fd %d", fd);
    httpd_sess_trigger_close(ws_keep_alive_get_user_ctx(h), fd);
    return true;
}

bool check_client_alive_cb(ws_keep_alive_t h, int fd) {
    ESP_LOGD(TAG, "Checking if client (fd=%d) is alive", fd);
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = ws_keep_alive_get_user_ctx(h);
    resp_arg->fd = fd;

    if (httpd_queue_work(resp_arg->hd, send_ping, resp_arg) == ESP_OK) {
        return true;
    }
    return false;
}

/* Setup and start server */
esp_err_t start_rest_server(const char *base_path) {
    /* FS path setup */
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    /* keep_alive engine */
    ws_keep_alive_config_t keep_alive_config = KEEP_ALIVE_CONFIG_DEFAULT();
    keep_alive_config.max_clients = max_clients;
    keep_alive_config.client_not_alive_cb = client_not_alive_cb;
    keep_alive_config.check_client_alive_cb = check_client_alive_cb;
    ws_keep_alive_t keep_alive = ws_keep_alive_start(&keep_alive_config);
    
    /* Start httpd server */
    ESP_LOGI(TAG, "Starting HTTPS Server");
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 0;
    config.global_user_ctx = keep_alive;
    config.open_fn = ws_open_fd;
    config.close_fn = ws_close_fd;
    config.max_open_sockets = max_clients;
    config.uri_match_fn = httpd_uri_match_wildcard;
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    /* Setup URI handlers */
    ESP_LOGI(TAG, "Registering URI handlers");

    httpd_uri_t ws_uri = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .user_ctx   = NULL,
        .is_websocket = true
    };
    httpd_register_uri_handler(server, &ws_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);
    ws_keep_alive_set_user_ctx(keep_alive, server);

    server_ptr = server;

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}
