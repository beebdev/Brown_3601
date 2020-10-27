#include "esp_err.h"
#include <stdint.h>


esp_err_t start_rest_server(const char *base_path);
esp_err_t ws_server_notify_all(uint32_t data);