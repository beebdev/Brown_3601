#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "brown_mqtt_client.h"

void mqtt_client_task(void *arg) {
    unsigned int cnt = 0;
    while (1) {
        cnt++;
        if (cnt % 100 == 0) {
            printf("mqtt_client\n");
            cnt = 0;
        }
        vTaskDelay(1);
    }
}