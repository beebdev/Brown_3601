/* Definitions */
#define GPIO_HANDSHAKE 2
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

/* FreeRTOS task for spi_slave */
void spi_slave_task(void *arg);