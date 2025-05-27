#ifndef __AT86RF215_H__
#define __AT86RF215_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "at86rf215_common.h"
#include "at86rf215_radio.h"

// Global structures - TX configuration ...
typedef struct{
    uint8_t tx_control_with_iq_if;
    at86rf215_radio_tx_pa_ramp_en pa_ramping_time;
    at86rf215_radio_pa_current_reduction_en current_reduction;
    int tx_power;
    at86rf215_radio_tx_cut_off_en analog_bw;
    at86rf215_radio_f_cut_en digital_bw;
    at86rf215_radio_sample_rate_en fs;
} at86rf215_tx_control_st; // Tx control top - at86rf215_tx_control_st ...    

// Global structures - RX configuration ...
typedef struct{
    at86rf215_radio_rx_bw_en radio_rx_bw;
    at86rf215_radio_f_cut_en digital_bw;
    at86rf215_radio_sample_rate_en fs;
    int agc_enable;
    int agc_gain;
    at86rf215_radio_agc_relative_atten_en agc_relative_atten;
    at86rf215_radio_agc_averaging_en agc_averaging;
} at86rf215_rx_control_st; // Rx control top - at86rf215_tx_control_st ...

int at86rf215_init(at86rf215_st* dev);
int at86rf215_close(at86rf215_st* dev, int reset_dev);
void at86rf215_reset(at86rf215_st* dev);
void at86rf215_chip_reset_with_spi(at86rf215_st* dev);

void at86rf215_get_versions(at86rf215_st* dev, uint8_t *pn, uint8_t *vn);
int at86rf215_print_version(at86rf215_st* dev);
void at86rf215_set_clock_output(at86rf215_st* dev,
                                at86rf215_drive_current_en drv_level,
                                at86rf215_clock_out_freq_en clock_val);
void at86rf215_setup_rf_irq(at86rf215_st* dev,  uint8_t active_low,
                                                uint8_t show_masked_irq,
                                                at86rf215_drive_current_en drive);
void at86rf215_set_xo_trim(at86rf215_st* dev, uint8_t fast_start, float cap_trim);
void at86rf215_get_iq_if_cfg(at86rf215_st* dev, at86rf215_iq_interface_config_st* cfg, int verbose);
void at86rf215_setup_iq_if(at86rf215_st* dev, at86rf215_iq_interface_config_st* cfg);
void at86rf215_setup_iq_radio_transmit(at86rf215_st* dev, at86rf215_rf_channel_en radio, uint64_t freq_hz, at86rf215_tx_control_st *tx_control,
                                         int iqloopback, at86rf215_iq_clock_data_skew_en skew);
void at86rf215_setup_iq_radio_receive(at86rf215_st *dev, at86rf215_rf_channel_en radio, uint64_t freq_hz, at86rf215_rx_control_st *rx_control,
                                        int iqloopback, at86rf215_iq_clock_data_skew_en skew);
void at86rf215_stop_iq_radio_receive (at86rf215_st* dev, at86rf215_rf_channel_en radio);
void at86rf215_setup_iq_radio_continues_tx (at86rf215_st* dev, at86rf215_rf_channel_en radio);
void at86rf215_setup_iq_radio_dac_value_override (at86rf215_st* dev, at86rf215_rf_channel_en ch,
                                                    uint32_t freq_hz,
                                                    uint8_t tx_power );
void at86rf215_setup_iq_radio_dac_value_override_no_freq (at86rf215_st* dev,
                                                          at86rf215_rf_channel_en ch,
                                                          uint8_t tx_power);
int64_t at86rf215_setup_channel ( at86rf215_st* dev, at86rf215_rf_channel_en ch, uint64_t freq_hz );
double at86rf215_check_freq (at86rf215_st* dev, at86rf215_rf_channel_en ch, uint64_t freq_hz );

void at86rf215_get_iq_sync_status(at86rf215_st* dev);

// EVENTS ...
void event_node_init(event_st* ev);
void event_node_close(event_st* ev);
void event_node_wait_ready(event_st* ev);
void event_node_signal_ready(event_st* ev, int ready);

#ifdef __cplusplus
}
#endif

#endif // __AT86RF215_H__
