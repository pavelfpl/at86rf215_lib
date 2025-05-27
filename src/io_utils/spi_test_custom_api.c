/*
 * Simple Linux wrapper for access to /dev/spidev
 * File: "spi_test.c" - test module
 */

#include "io_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//-----------------------------------------------------------------------------
#define SPI_DEVICE "/dev/spidev0.0"   // or #define SPI_DEVICE "/dev/spidev1.0"
#define GPIO_DEVICE "/dev/gpiochip2"
//-----------------------------------------------------------------------------
#define GPIO_CS_OFFSET 0
#define GPIO_RESET_OFFSET 1 
#define GPIO_EXT_INT_OFFSET 2
// ----------------------------------------------------------------------------
/* --  Version register -- */
#define REG_RF_PN 0x000D
#define REG_RF_VN 0x000E

typedef struct at86rf215_st_t{
       int version;
       spi_t *io_spi;
}at86rf215_st;

unsigned int thread_cnt = 0;

// Function prototypes
static int at86rf215_write_buffer(at86rf215_st* dev, uint16_t addr, uint8_t *buffer, uint8_t size);
static int at86rf215_read_buffer(at86rf215_st* dev, uint16_t addr, uint8_t *buffer, uint8_t size);
static int at86rf215_write_byte(at86rf215_st* dev, uint16_t addr, uint8_t val);
static int at86rf215_read_byte(at86rf215_st* dev, uint16_t addr);
void at86rf215_get_versions(at86rf215_st* dev, uint8_t *pn, uint8_t *vn);
void at86rf215_get_versions_2(at86rf215_st* dev, uint8_t *pn, uint8_t *vn);
int at86rf215_print_version(at86rf215_st* dev);

static int at86rf215_write_buffer(at86rf215_st* dev, uint16_t addr, uint8_t *buffer, uint8_t size ){
    
    // A maximal possible chunk size - 256 + 2(addr)
    uint8_t chunk_tx[256] = {0};
    addr = (addr & 0x3FFF) | 0x8000;
    
    memcpy(chunk_tx, buffer, size);

    return io_utils_spi_write_buffer(dev->io_spi, addr, chunk_tx, size);
}

static int at86rf215_read_buffer(at86rf215_st* dev, uint16_t addr, uint8_t *buffer, uint8_t size){
    
    // A maximal possible chunk size - 256 + 2(addr)
    uint8_t chunk_rx[256] = {0};
    addr = (addr & 0x3FFF);

    int ret = io_utils_spi_read_buffer(dev->io_spi, addr, chunk_rx, size);

    if (ret > 0){
        memcpy(buffer, chunk_rx, size);
    }
    
    return ret;
}

static int at86rf215_write_byte(at86rf215_st* dev, uint16_t addr, uint8_t val){
    
    uint8_t chunk_tx = val;
    addr = (addr & 0x3FFF) | 0x8000;
    
    return io_utils_spi_write_byte(dev->io_spi, addr, chunk_tx);
}

static int at86rf215_read_byte(at86rf215_st* dev, uint16_t addr){

    uint8_t chunk_rx = {0};
    addr = (addr & 0x3FFF);
    
    int ret = io_utils_spi_read_byte(dev->io_spi, addr, &chunk_rx);
    
    if (ret < 0){
        return ret;
    }
    
    return chunk_rx;
}

void at86rf215_get_versions(at86rf215_st* dev, uint8_t *pn, uint8_t *vn){
    
    if (pn) *pn = at86rf215_read_byte(dev, REG_RF_PN);
    if (vn) *vn = at86rf215_read_byte(dev, REG_RF_VN);
    
}

void at86rf215_get_versions_2(at86rf215_st* dev, uint8_t *pn, uint8_t *vn){
    
    uint8_t buf[2] = {0};
    
    at86rf215_read_buffer(dev,REG_RF_PN,buf,2);
    
    *pn = buf[0];
    *vn = buf[1];
    
}

int at86rf215_print_version(at86rf215_st* dev){
    
	uint8_t pn = 0, vn = 0;
	// at86rf215_get_versions(dev, &pn, &vn);
    at86rf215_get_versions_2(dev, &pn, &vn);
    
    printf("MODEM product: 0x%02x\n", pn);
    
	if (pn == 0x34) 
    {
        printf("MODEM Version: AT86RF215 (with basebands), version: 0x%02x\n", vn);
    }
    else if (pn == 0x35)
    {
        printf("MODEM Version: AT86RF215IQ (without basebands), version: 0x%02x\n", vn);
    }
    else
    {
        printf("MODEM Version: not AT86RF215 IQ capable modem (product number: 0x%02x, versions 0x%02x)\n", pn, vn);
    }
    
	return pn;
}

static void gpio_event_callback(void *param, void *user_data){
    
    struct at86rf215_st_t *data_ptr = (struct at86rf215_st_t *)user_data;
    
    // Test event type ...
    if(*(int *)param == GPIOEVENT_EVENT_RISING_EDGE){
       printf("Hit callback - GPIO event rising edge ...\n");    
    }
    
    if(*(int *)param == GPIOEVENT_EVENT_FALLING_EDGE){
       printf("Hit callback - GPIO event falling edge ...\n");    
    }
    
    // Show user data ...
    if(data_ptr != NULL){
       printf("User data (=[at86rf215_st_t->version]: %d \n", data_ptr->version);
    }
    
    thread_cnt++;
    
}

int main(){
   
  at86rf215_st rf_struct ={0}; 
  rf_struct.version = 0x9999;  // Test value only ...
    
  gpio_list(GPIO_DEVICE);
  
  // Init GPIO bank + SPI CS  
  io_utils_setup_gpio(GPIO_DEVICE, GPIO_CS_OFFSET); 
    
  // Init SPI + spi struct
  int ret = io_utils_spi_init(&rf_struct.io_spi, SPI_DEVICE, 0, 0, 1000000);
  
  if(ret!=0){
     printf("Unable to init SPI device !!!\n");
     return 1; 
  }
  
  // Print version of AT86RF215 ...
  at86rf215_print_version(&rf_struct);
  
  // Reset GPIO to 0
  io_utils_write_gpio(GPIO_RESET_OFFSET, GPIO_LO_LEVEL);
  
  // Setup external interrupt callback
  ret = io_utils_setup_interrupt(GPIO_EXT_INT_OFFSET, &gpio_event_callback, (void *)&rf_struct);
  
  if(ret!=0){
     printf("Unable to set externall interrupt !!!\n");
     return 1; 
  }
  
  for(int i=0;i<60;i++){
     // io_utils_write_gpio --> toggle 0/1
     io_utils_write_gpio(GPIO_RESET_OFFSET, GPIO_HI_LEVEL);
     sleep(2);
     io_utils_write_gpio(GPIO_RESET_OFFSET, GPIO_LO_LEVEL);
  }
  
  // Show callback status:
  printf("Callback function was called n-times %d: \n",thread_cnt);
  
  // Sleep for 60 seconds ...
  sleep(60);
  
  // Disable external interrupt
  io_utils_disable_interrupt();
  
  // Close SPI      
  io_utils_spi_close(rf_struct.io_spi);      
  
  return 0;
}

/*** end of "spi_test.c" file ***/

