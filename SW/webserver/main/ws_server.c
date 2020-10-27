#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"

#include <esp_http_server.h>

static const char *TAG = "ws_server";
static const size_t max_clients = 4;
static httpd_handle_t server = NULL;

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
    uint32_t data;
};

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t ws_handler(httpd_req_t *req)
{
    uint8_t buf[128] = { 0 };
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;

    /* Receive full message first */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }

    // /* If it was a PONG, update keep alive */
    // if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
    //     ESP_LOGD(TAG, "Recieved PONG message");
    //     return ws_keep_alive_client_is_active(httpd_get_global_user_ctx(req->handle),
    //             httpd_req_to_sockfd(req));
    // /* If it was a TEXT message, just echo it back */
    // } else 
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
        ESP_LOGI(TAG, "ws_handler: Received packet with message: %s", ws_pkt.payload);
        // ret = httpd_ws_send_frame(req, &ws_pkt);
        // if (ret != ESP_OK) {
        //     ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
        // }
        ESP_LOGI(TAG, "ws_handler: httpd_handle_t=%p, sockfd=%d, client_info:%d", req->handle,
                 httpd_req_to_sockfd(req), httpd_ws_get_fd_info(req->handle, httpd_req_to_sockfd(req)));
        return ret;
    }
    return ESP_OK;
}

// esp_err_t ws_open_fd(httpd_handle_t hd, int fd) {
//     ESP_LOGI(TAG, "New client connected &d", fd);
//     ws_keep_alive_t h = httpd_get_global_user_ctx(hd);
//     return ws_keep_alive_add_client(h, fd);
// }

// void ws_close_fd(httpd_handle_t hd, int fd) {
//     ESP_LOGI(TAG, "Client disconnected %d", fd);
//     ws_keep_alive_t h = httpd_get_global_user_ctx(hd);
//     return wss_keep_alive_remove_client(h, fd);
// }

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static void send_data(void *arg) {
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    uint32_t data = resp_arg->data;
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *) &data;
    ws_pkt.len = 4;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

// static void send_ping(void *arg) {
//     struct async_resp_arg *resp_arg = arg;
//     httpd_handle_t hd = resp_arg->hd;
//     int fd = resp_arg->fd;
    
//     httpd_ws_frame_t = ws_pkt;
//     memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
//     ws_pkt.payload = NULL;
//     ws_pkt.len = 0;
//     ws_pkt.type = HTTPD_WS_TYPE_PING;

//     httpd_ws_send_frame_async(hd, fd, &ws_pkt);
//     free(resp_arg);
// }

// bool client_not_alive_cb(ws_keep_alive_t h, int fd) {
//     ESP_LOGE(TAG, "Client not alive, closing fd=%d", fd);
//     httpd_sess_trigger_close(ws_keep_alive_get_user_ctx(h), fd);
//     return true;
// }

// bool check_client_alive_cb(ws_keep_alive_t h, int fd) {
//     ESP_LOGD(TAG, "Checking if client (fd=%d) is alive", fd);
//     struct async_resp_arg *resp_arg = malloc(sizeof(async_resp_arg));
//     resp_arg->hd = ws_keep_alive_get_user_ctx(h);
//     resp_arg->fd = fd;
//     resp_arg->data = 0;

//     if (httpd_queue_work(resp_arg->hd, send_ping, resp_arg) == ESP_OK) {
//         return true;
//     }

//     return false;
// }

static httpd_handle_t start_webserver(void) {
    /* Keep alive engine */
    // ws_keep_alive_config_t ka_config = KEEP_ALIVE_CONFIG_DEFAULT();
    // ka_config.max_clients = max_clients;
    // ka_config.client_not_alive_cb = client_not_alive_cb;
    // ka_config.check_client_alive_cb = check_client_alive_cb;
    // ws_keep_alive_t ka = ws_keep_alive_start(&ka_config);

    /* Start ws server */
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 1;
    // config.global_user_ctx = keep_alive;
    // config.open_fn = ws_open_fd;
    // config.close_fn = ws_close_fd;

    /* Start the httpd server */
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Registering the ws handler */
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        // ws_keep_alive_set_user_ctx(ka, server);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server) {
    /* Stop keep alive engine */
    // wss_keep_alive_stop(httpd_get_global_user_ctx(server));
    /* Stop the httpd server */
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

/* Get all clients and send async message */
void ws_server_notify_all(uint32_t data) {
    if (server == NULL) {
        ESP_LOGE(TAG, "ws server not setup");
        return;
    }
    // ESP_LOGI(TAG, "ws server: %d", server==NULL);
    // while (server == NULL); /* Wait until server is created */

    size_t clients = max_clients;
    int client_fds[max_clients];
    if (httpd_get_client_list(server, &clients, client_fds) == ESP_OK) {
        for (size_t i = 0; i < clients; i++) {
            int sd = client_fds[i];
            if (httpd_ws_get_fd_info(server, sd) == HTTPD_WS_CLIENT_WEBSOCKET) {
                ESP_LOGI(TAG, "Active client (fd=%d) -> sending async message", sd);
                struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
                resp_arg->hd = server;
                resp_arg->fd = sd;
                resp_arg->data = data;

                if (httpd_queue_work(resp_arg->hd, send_data, resp_arg) != ESP_OK) {
                    ESP_LOGE(TAG, "httpd_queue_work failed");
                    return;
                }
            }
        }
    } else {
        ESP_LOGE(TAG, "httpd_get_client_list failed");
        return;
    }
}

void start_ws_server(void) {
    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET

    /* Start the server for the first time */
    server = start_webserver();
    uint16_t cntr = 0;
    while(1) {
        cntr++;
        if (cntr%1000 == 0) {
            vTaskDelay(1);
            cntr = 0;
        }
    }
}
