/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_err.h"

#include "brown_mqtt_client.h"
#include "brown_spi_slave.h"
#include "brown_webserver.h"

void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(webserver_task, "web_server", 4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(spi_slave_task, "spi_slave", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(mqtt_client_task, "mqtt_client", 4096, NULL, 3, NULL, 1);
}
