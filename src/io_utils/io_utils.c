#include "io_utils.h"
#include <string.h>

#define GPIO_EXT_IS_TIMEOUT 10000000

// Internal structure to store midleware data 
typedef struct gpio_settings_t{
    char *gpio_dev_name;
    char *gpio_dev_name_isr;
    int gpio_cs_offset;
    int gpio_irq_offset;
    int p_call_param;
}gpio_settings_s;

static spi_t spi_int;

static gpio_settings_s gpio_set_int = {0};

// io_utils_setup_gpio - setup gpio structure and CS for SPI
void io_utils_setup_gpio(char *gpio_dev_name,char * gpio_dev_name_isr, int gpio_cs_offset){
    
    memset(&gpio_set_int, 0, sizeof(gpio_set_int));
    
    gpio_set_int.gpio_dev_name = gpio_dev_name;            // Devive gpio name - e.g: /dev/gpiochip2 
    gpio_set_int.gpio_cs_offset = gpio_cs_offset;          // CS GPIO offset - standard GPIO port is used to emulate SPI chip select 
    
    gpio_set_int.gpio_dev_name_isr = gpio_dev_name_isr;    // Devive gpio name - e.g: /dev/gpiochip0 
    
    // Set CS to "HI" level by default ...
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_HI_LEVEL);
    
}

// io_utils_write_gpio - set GPIO pin as output + level
void io_utils_write_gpio(int gpio_offset, uint8_t level){
        
     // Set selected GPIO as output with given level - GPIO_HI_LEVEL / GPIO_LO_LEVEL
     gpio_write_single(gpio_set_int.gpio_dev_name, gpio_offset, level);
    
}

// io_utils_setup_interrupt - setup external interrupt and callback function + user data 
int io_utils_setup_interrupt(int gpio_offset, void (* p_callback)(void *user_param, void *user_data), void *p_user_data){
    
    int ret_val = 0;
    
    // Check for external GPIO interrupt events (GPIOEVENT_EVENT_RISING_EDGE - default or GPIOEVENT_EVENT_FALLING_EDGE ) ...
    ret_val = gpio_poll_thread_start(gpio_set_int.gpio_dev_name_isr, gpio_offset, GPIOEVENT_EVENT_RISING_EDGE , GPIO_EXT_IS_TIMEOUT , p_callback, (void *)&gpio_set_int.p_call_param, p_user_data);
    
    return ret_val;
}

// io_utils_disable_interrupt - disable external interrupt 
void io_utils_disable_interrupt(){
    
    // Disable polling interrupts 
    gpio_poll_thread_stop();
    
}

// io_utils_usleep - sleep for uses (using nanosleep)
void io_utils_usleep(int usec){
    
    struct timespec req = {.tv_sec = 0, .tv_nsec = usec * 1000L};
    nanosleep(&req, (struct timespec *)NULL);
    
}

// io_utils_spi_init - init spi bus
int io_utils_spi_init(spi_t **spi, const char *device, int mode, int bits, int speed){
    
    int ret_val = 0;
    
    if(gpio_set_int.gpio_dev_name == NULL) return -1;
    
    // Init spi device ...
    ret_val = spi_init(&spi_int,
                    device, // filename like "/dev/spidev0.0"
                    mode,   // SPI_* (look "linux/spi/spidev.h")
                    bits,   // bits per word (usually 8)
                    speed); // max speed [Hz]
    
    if(ret_val != 0){
       *spi = 0;  
    }else{
       *spi = &spi_int;    
    }
    
    return ret_val;

}

// io_utils_spi_close - close spi bus
void io_utils_spi_close(spi_t *spi){
    
    // Deinit spi device
    spi_free(spi); spi = 0;
}

// io_utils_spi_read_buffer - spi read buffer
int io_utils_spi_read_buffer(spi_t *spi, uint16_t addr, uint8_t *buffer, uint8_t size){
    
    int ret_val = 0;
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_LO_LEVEL);
    
    // Read SPI register wrapper ...
    ret_val = spi_read_reg16(spi, addr, buffer, size, 1);   // Address byte swap is enabled ...
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_HI_LEVEL);

    return ret_val;
}

// io_utils_spi_write_buffer - spi write buffer
int io_utils_spi_write_buffer(spi_t *spi, uint16_t addr, uint8_t *buffer, uint8_t size){
    
    int ret_val = 0;
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_LO_LEVEL);
    
    // Read SPI register wrapper ...  
    ret_val = spi_write_reg16(spi, addr, buffer, size, 1);  // Address byte swap is enabled ...
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_HI_LEVEL);
    
    return ret_val;

}

// io_utils_spi_read_byte - spi read byte
int io_utils_spi_read_byte(spi_t *spi, uint16_t addr, uint8_t *byte){
    
    int ret_val = 0;
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_LO_LEVEL);
    // Read SPI register wrapper ...
    
    ret_val = spi_read_reg16(spi, addr, byte, 1, 1);   // Address byte swap is enabled ...
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_HI_LEVEL);
    
    return ret_val;
}

// io_utils_spi_write_byte - spi write byte
int io_utils_spi_write_byte(spi_t *spi, uint16_t addr, uint8_t byte){
    
    int ret_val = 0;
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_LO_LEVEL);
    // Read SPI register wrapper ...  
    ret_val = spi_write_reg16(spi, addr, &byte, 1, 1);  // Address byte swap is enabled ...
    
    gpio_write_single(gpio_set_int.gpio_dev_name, gpio_set_int.gpio_cs_offset, GPIO_HI_LEVEL);
    
    return ret_val;

}

