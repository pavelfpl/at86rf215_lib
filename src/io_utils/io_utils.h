/**
 * io_utils - middleware for GPIO + SPI 
 * Pavel Fiala @ 2024
 * h. file
 **/

#ifndef IO_UTILS_H_
#define IO_UTILS_H_

#include "gpiodev_lib.h"
#include "spi.h"

#include <stdint.h>

#define GPIO_HI_LEVEL 1
#define GPIO_LO_LEVEL 0

void io_utils_setup_gpio(char *gpio_dev_name,char * gpio_dev_name_isr, int gpio_cs_offset);
void io_utils_write_gpio(int gpio_offset, uint8_t level);
int io_utils_setup_interrupt(int gpio_offset, void (* p_callback)(void *user_param, void *user_data), void *p_user_data);
void io_utils_disable_interrupt();
void io_utils_usleep(int usec);
int io_utils_spi_init(spi_t **spi, const char *device, int mode, int bits, int speed);
void io_utils_spi_close(spi_t *spi);
int io_utils_spi_read_buffer(spi_t *spi, uint16_t addr, uint8_t *buffer, uint8_t size);
int io_utils_spi_write_buffer(spi_t *spi, uint16_t addr, uint8_t *buffer, uint8_t size);
int io_utils_spi_read_byte(spi_t *spi, uint16_t addr, uint8_t *byte);
int io_utils_spi_write_byte(spi_t *spi, uint16_t addr, uint8_t byte);

#endif
