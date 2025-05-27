/**
 * GPIO library for /dev/gpiodevX
 * Pavel Fiala @ 2024
 * h. file
 **/

#ifndef GPIO_LIB_H_
#define GPIO_LIB_H_

#include <linux/gpio.h>
#include <time.h>

#include <time.h>
#include <stdint.h>

void gpio_list(const char *dev_name);
void gpio_write_single(const char *dev_name, int offset, uint8_t value);
int gpio_read_single(const char *dev_name, int offset, uint8_t *value);
int gpio_poll_wait(const char *dev_name, int offset, int timeout, uint32_t event_flags);
int gpio_poll_thread_start(char *dev_name, int offset, uint32_t event_flags,time_t wait_time, void (* p_callback)(void *user_param, void *user_data), void *p_param, void *p_user_data);
void gpio_poll_thread_stop();

#endif
