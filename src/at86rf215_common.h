#ifndef __AT86RF215_COMMON_H__
#define __AT86RF215_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>    
    
#include "io_utils/io_utils.h"
#include "at86rf215_regs.h"

#pragma pack(1)

typedef enum
{
    at86rf215_rf_channel_900mhz = 0,
    at86rf215_rf_channel_2400mhz = 1,
} at86rf215_rf_channel_en;

typedef struct
{
    uint8_t wake_up_por:1;
    uint8_t trx_ready:1;
    uint8_t energy_detection_complete:1;
    uint8_t battery_low:1;
    uint8_t trx_error:1;
    uint8_t IQ_if_sync_fail:1;
    uint8_t res :2;
} at86rf215_radio_irq_st;

typedef struct
{
    uint8_t frame_rx_started:1;
    uint8_t frame_rx_complete:1;
    uint8_t frame_rx_address_match:1;
    uint8_t frame_rx_match_extended:1;
    uint8_t frame_tx_complete:1;
    uint8_t agc_hold:1;
    uint8_t agc_release :1;
    uint8_t frame_buffer_level :1;
} at86rf215_baseband_irq_st;

typedef struct
{
    at86rf215_radio_irq_st radio09;
    at86rf215_radio_irq_st radio24;
    at86rf215_baseband_irq_st bb0;
    at86rf215_baseband_irq_st bb1;
} at86rf215_irq_st;

#pragma pack()

typedef enum
{
    at86rf215_iq_drive_current_1ma = 0,
    at86rf215_iq_drive_current_2ma = 1,
    at86rf215_iq_drive_current_3ma = 2,
    at86rf215_iq_drive_current_4ma = 3,
} at86rf215_iq_drive_current_en;

typedef enum
{
    at86rf215_iq_common_mode_v_150mv = 0,
    at86rf215_iq_common_mode_v_200mv = 1,
    at86rf215_iq_common_mode_v_250mv = 2,
    at86rf215_iq_common_mode_v_300mv = 3,
    at86rf215_iq_common_mode_v_ieee1596_1v2 = 4,
} at86rf215_iq_common_mode_v_en;

typedef enum
{
    at86rf215_baseband_mode = 0,
    at86rf215_iq_if_mode = 1,
} at86rf215_baseband_iq_mode_en;

typedef enum
{
    at86rf215_iq_clock_data_skew_1_906ns = 0,
    at86rf215_iq_clock_data_skew_2_906ns = 1,
    at86rf215_iq_clock_data_skew_3_906ns = 2,
    at86rf215_iq_clock_data_skew_4_906ns = 3,
} at86rf215_iq_clock_data_skew_en;

typedef struct
{
    uint8_t loopback_enable;
    at86rf215_iq_drive_current_en drv_strength;
    at86rf215_iq_common_mode_v_en common_mode_voltage;
    uint8_t tx_control_with_iq_if;
    at86rf215_baseband_iq_mode_en radio09_mode;
    at86rf215_baseband_iq_mode_en radio24_mode;
    at86rf215_iq_clock_data_skew_en clock_skew;

    // status
    uint8_t synchronization_failed;
    uint8_t in_failsafe_mode;
    uint8_t synchronized_incoming_iq;
} at86rf215_iq_interface_config_st;

typedef struct
{
    int low_ch_i;
    int low_ch_q;
    int hi_ch_i;
    int hi_ch_q;
} at86rf215_cal_results_st;


typedef struct 
{
    pthread_mutex_t ready_mutex;
    pthread_cond_t ready_cond;
    int ready;
} event_st;

typedef struct
{
    event_st lo_trx_ready_event;
    event_st lo_energy_measure_event;
    event_st hi_trx_ready_event;
    event_st hi_energy_measure_event;
} at86rf215_events_st;

typedef struct
{
    // Pinout ...
    int cs_pin;    // CS PIN offset
    int reset_pin; // RESET pin offset
	int irq_pin;   // IRQ pin

    // SPI device ...
    int spi_mode;  // SPI mode
    int spi_bits;  // SPI bits 
    int spi_speed; // SPI baud rate

    // internal controls
    spi_t *io_spi; // SPI handle

    int initialized;              // Initialized status 
    at86rf215_cal_results_st cal; // Cal. status
    bool override_cal;            // Overriade cal
    at86rf215_events_st events;   // Events
	int num_interrupts;           // Num interrupts happen 
} at86rf215_st;


int at86rf215_write_buffer(at86rf215_st* dev, uint16_t addr, uint8_t *buffer, uint8_t size );
int at86rf215_read_buffer(at86rf215_st* dev, uint16_t addr, uint8_t *buffer, uint8_t size);
int at86rf215_write_byte(at86rf215_st* dev, uint16_t addr, uint8_t val );
int at86rf215_read_byte(at86rf215_st* dev, uint16_t addr);
void at86rf215_interrupt_handler (void *param, void *user_data);
int at86rf215_write_fifo(at86rf215_st* dev, uint8_t *buffer, uint8_t size );
int at86rf215_read_fifo(at86rf215_st* dev, uint8_t *buffer, uint8_t size );
void at86rf215_get_irqs(at86rf215_st* dev, at86rf215_irq_st* irq, int verbose);

#ifdef __cplusplus
}
#endif

#endif // __AT86RF215_COMMON_H__
