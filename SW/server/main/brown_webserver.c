#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "brown_webserver.h"

void webserver_task(void *arg) {
    unsigned int cnt = 0;
    while (1) {
        cnt++;
        if (cnt % 100 == 0) {
            printf("webserver\n");
            cnt = 0;
        }
        vTaskDelay(1);
    }
}