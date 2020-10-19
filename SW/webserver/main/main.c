/* SPI slave
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// lwip?

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "soc/rtc_periph.h"
#include "driver/spi_slave.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

/* Definitions */
#define GPIO_HANDSHAKE 2
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void my_post_setup_cb(spi_slave_transaction_t *trans) {
    WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<GPIO_HANDSHAKE));
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void my_post_trans_cb(spi_slave_transaction_t *trans) {
    WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1<<GPIO_HANDSHAKE));
}

void app_main(void)
{
    int n = 0;
    esp_err_t ret;

    /* SPI bus config */
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
    };

    /* SPI slave interface config */
    spi_slave_interface_config_t slvcfg = {
        .mode = 1,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
        .post_setup_cb = my_post_setup_cb,
        .post_trans_cb = my_post_trans_cb
    };

    /* GPIO config */
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << GPIO_HANDSHAKE)
    };

    /* Config handshake line as output */
    gpio_config(&io_conf);

    /* Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected */
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    /* Initialise SPI slave interface */
    ret = spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, 1);
    assert(ret == ESP_OK);

    WORD_ALIGNED_ATTR uint16_t recvbuf[2];
    WORD_ALIGNED_ATTR uint16_t sendbuf[2];
    memset(recvbuf, 0, 4);
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    printf("Configuration Done!\n");

    while (1) {
        printf("[Slave] Entered loop\n");
        memset(recvbuf, 0, 4);
        memset(sendbuf, 0xAA, 4);

        /* Setup a transaction of 128 bytes to send/recv */
        t.length = 32;
        t.trans_len = 32; 
        t.rx_buffer = recvbuf;
        t.tx_buffer = sendbuf;
        
        printf("[Slave] Starting transaction...");
        ret = spi_slave_transmit(HSPI_HOST, &t, portMAX_DELAY);
        printf("Done\n");
        
        printf("Received: %04x %04x\n", recvbuf[0], recvbuf[1]);
        n++;
    }
}
