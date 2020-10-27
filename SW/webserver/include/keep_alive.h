#pragma once

#define KEEP_ALIVE_CONFIG_DEFAULT()         \
    {                                       \
    .max_clients = 10,                      \
    .task_stack_size = 2048,                \
    .task_prio = tskIDLE_PRIORITY+1,        \
    .keep_alive_period_ms = 3000,           \
    .not_alive_after_ms = 6000,            \
}

struct ws_keep_alive_storage;
typedef struct ws_keep_alive_storage* ws_keep_alive_t;
typedef bool (*ws_check_client_alive_cb_t)(ws_keep_alive_t h, int fd);
typedef bool (*ws_client_not_alive_cb_t)(ws_keep_alive_t h, int fd);

/**
 * @brief Confiuration struct
 */
typedef struct {
    size_t max_clients;                                      /*!< max number of clients */
    size_t task_stack_size;                                  /*!< stack size of the created task */
    size_t task_prio;                                        /*!< priority of the created task */
    size_t keep_alive_period_ms;                             /*!< check every client after this time */
    size_t not_alive_after_ms;                               /*!< consider client not alive after this time */
    ws_check_client_alive_cb_t check_client_alive_cb;        /*!< callback function to check if client is alive */
    ws_client_not_alive_cb_t client_not_alive_cb;            /*!< callback function to notify that the client is not alive */
    void *user_ctx;                                          /*!< user context available in the keep-alive handle */
} ws_keep_alive_config_t;

/**
 * @brief Adds a new client to internal set of clients to keep an eye on
 *
 * @param h keep-alive handle
 * @param fd socket file descriptor for this client
 * @return ESP_OK on success
 */
esp_err_t ws_keep_alive_add_client(ws_keep_alive_t h, int fd);

/**
 * @brief Removes this client from the set
 *
 * @param h keep-alive handle
 * @param fd socket file descriptor for this client
 * @return ESP_OK on success
 */
esp_err_t ws_keep_alive_remove_client(ws_keep_alive_t h, int fd);

/**
 * @brief Notify that this client is alive
 *
 * @param h keep-alive handle
 * @param fd socket file descriptor for this client
 * @return ESP_OK on success
 */

esp_err_t ws_keep_alive_client_is_active(ws_keep_alive_t h, int fd);

/**
 * @brief Starts keep-alive engine
 *
 * @param config keep-alive configuration
 * @return keep alive handle
 */
ws_keep_alive_t ws_keep_alive_start(ws_keep_alive_config_t *config);

/**
 * @brief Stops keep-alive engine
 *
 * @param h keep-alive handle
 */
void ws_keep_alive_stop(ws_keep_alive_t h);

/**
 * @brief Sets user defined context
 *
 * @param h keep-alive handle
 * @param ctx user context
 */
void ws_keep_alive_set_user_ctx(ws_keep_alive_t h, void *ctx);

/**
 * @brief Gets user defined context
 *
 * @param h keep-alive handle
 * @return ctx user context
 */
void* ws_keep_alive_get_user_ctx(ws_keep_alive_t h);

esp_err_t ws_get_client_list(ws_keep_alive_t h, int *client_fds);