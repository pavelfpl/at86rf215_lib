# AT86RF215 library for Altera/Intel SocFPGA

This library is highly modified fork of: https://github.com/cariboulabs/cariboulite/tree/main/software/libcariboulite (comming from Raspberry PI)  
Primary support is for Altera/Intel SocFPGA (Cyclone V SoC, Arria 10, Stratix 10, ...), but it may be used with common Linux spidev and gpiodev devices.
SocFPGA is factically using common Designware IP cores for both SPI (master) and GPIO devices.  

## Library features:
- I/Q mode with TX/RX support, extended support for TX mode
- io_utils (user space HW layer) rewritten for SocFPGA support - SPI layer ("snps,dw-apb-ssi") and GPIO layer ("snps,dw-apb-gpio") drivers
- SPI Chip select pin is emulated by standard GPIO to prevent CS going up between bytes when using CS driven by SPI core (hack)
- external IRQ from AT86RF215 is supported
- two test binaries are created: test_at86rf215_rx (for receive testing) and test_at86rf215_rx (for transmit test)
- shared (.so) library is created and installed; can be used with custom GnuRadio modules etc.

## How to build library

```
git clone https://github.com/pavelfpl/at86rf215_lib
cd at86rf215_lib
mkdir buid
cd build
cmake ../
# Custom install option:
or cmake ../ -DCMAKE_INSTALL_PREFIX=/home/user/lib_at86rf215
make 
sudo make install # or make install if you are using user directory
sudo ldconfig

These items are installed:

# -- Installing: /usr/local/bin/test_at86rf215_rx
# -- Installing: /usr/local/bin/test_at86rf215_tx
# -- Installing: /usr/local/lib/lib_at86rf215_lib.so
# -- Installing: /usr/local/include/at86rf215.h
# -- Installing: /usr/local/include/at86rf215_regs.h
# -- Installing: /usr/local/include/at86rf215_radio.h
# -- Installing: /usr/local/include/at86rf215_common.h
# -- Installing: /usr/local/include/at86rf215_baseband.h
# -- Installing: /usr/local/include/zf_log/zf_log.h
# -- Installing: /usr/local/include/io_utils/io_utils.h
# -- Installing: /usr/local/include/io_utils/gpiodev_lib.h
# -- Installing: /usr/local/include/io_utils/spi.h

Uninstall support: 
sudo make uninstall

If you install library to custom path - example:
export LD_LIBRARY_PATH="/home/user/lib_at86rf215/lib:$LD_LIBRARY_PATH"

```
## How to use library / test programs as normal user:
```
# Setup following udev rules: /etc/udev/rules.d/99-gpiodev_spidev.rules 

# udevadm info -a -n /dev/spidev0.0
KERNEL=="spidev0.0", SUBSYSTEM=="spidev", MODE="0666"
# udevadm info -a -n /dev/gpiochip0,1,2, ...
KERNEL=="gpiochip[0-9]*", SUBSYSTEM=="gpio", DRIVER=="gpio_stub_drv", MODE="0666"

```

## Output of the test_at86rf215_rx module

```
# Example output of:
./test_at86rf215_rx -f 2400000000 -e 0 -g 0x15 -s 400 -d 0_5 -a 800
# Options:
# --------
# -f --> supported frequency (in Hz), see at86rf215 datasheet for valid values
# -e --> AGC enable (1) / disable (0)
# -g --> RX gain in case of AGC is disabled, see at86rf215 datasheet for valid values
# -s --> sample rate - scaled value - 400 --> 400e3 = 400KSPS as minimum, 4000 --> 4000e3 = 4MSPS as maximum, see at86rf215 datasheet for valid values
# -d --> digital bandwidth, as string, "0_5" means 0.5 --> 0.5 *fS/2, see at86rf215 datasheet for valid values
# -a --> analog bandwidth - scaled value - 200 --> BW200KHZ, see at86rf215 datasheet for valid values

---------------------------- Sample Output of test_at86rf215_rx  ---------------------

2-14 11:41:42.182   568   568 D AT86RF215_Main at86rf215_init@at86rf215.c:168 Configuring reset and CS pins
02-14 11:41:42.183   568   568 D AT86RF215_Main at86rf215_init@at86rf215.c:179 Configuring SPI device modem
02-14 11:41:42.184   568   568 D AT86RF215_Main at86rf215_init@at86rf215.c:190 Configuring external interrupts
02-14 11:41:42.185   568   568 D AT86RF215_Main at86rf215_init@at86rf215.c:212 Modem identity: Version: 03, Product: 34
02-14 11:41:42.185   568   568 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:123 Calibration of modem channel 0...
02-14 11:41:42.256   568   568 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:139 Calibration Results of the modem: I=35, Q=25
02-14 11:41:42.256   568   568 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:123 Calibration of modem channel 1...
02-14 11:41:42.326   568   568 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:139 Calibration Results of the modem: I=27, Q=29
TEST:AT86RF215:VERSIONS:PN=0x34
TEST:AT86RF215:VERSIONS:VN=3
TEST:AT86RF215:VERSIONS:PASS=1                                                                                                                                                    
TEST:AT86RF215:VERSIONS:INFO=The component PN is AT86RF215 (0x34), Version 3                                                                                                      
-- AT86RF215 RX mode test performed --                                                                                                                                            
at86rf215 RF frequency: 2400000000l                                                                                                                                               
at86rf215 AGC enable: 0                                                                                                                                                           
at86rf215 RX gain: 0x0                                                                                                                                                            
at86rf215 RX gain: 0x15                                                                                                                                                           
at86rf215 sample rate: 400                                                                                                                                                        
at86rf215 digital bandwidth: 0_5                                                                                                                                                  
at86rf215 analog bandwidth: 800                                                                                                                                                   
Applying configuration for 2.4GHz ...                                                                                                                                             
02-14 11:41:42.329   568   568 D AT86RF215_Main at86rf215_setup_iq_radio_receive@at86rf215.c:753 Switching to TX/RX (transceiver) preparation mode  ...
02-14 11:41:42.340   568   569 D AT86RF215_Events at86rf215_interrupt_handler@at86rf215_events.c:150 Hit callback - start ...
02-14 11:41:42.340   568   569 D AT86RF215_Events at86rf215_interrupt_handler@at86rf215_events.c:154 Hit callback - GPIO event rising edge
IRQ Status:
  Radio09:
    IQ_if_sync_fail: 0
    trx_error: 0
    battery_low: 0
    energy_detection_complete: 0
    trx_ready: 0
    wake_up_por: 0
  Radio24:
    IQ_if_sync_fail: 0
    trx_error: 0
    battery_low: 0
    energy_detection_complete: 0
    trx_ready: 1
    wake_up_por: 0
  Baseband09:
    frame_buffer_level: 0
    agc_release: 0
    agc_hold: 0
    frame_tx_complete: 0
    frame_rx_match_extended: 0
    frame_rx_address_match: 0
    frame_rx_complete: 0
    frame_rx_started: 0
  Baseband24:
    frame_buffer_level: 0
    agc_release: 0
    agc_hold: 0
    frame_tx_complete: 0
    frame_rx_match_extended: 0
    frame_rx_address_match: 0
    frame_rx_complete: 0
    frame_rx_started: 0
02-14 11:41:42.341   568   569 D AT86RF215_Events at86rf215_radio_event_handler@at86rf215_events.c:61 INT @ RADIO24: Transceiver ready
02-14 11:41:42.341   568   568 D AT86RF215_Main at86rf215_setup_iq_radio_receive@at86rf215.c:760 Get hi_trx_ready_event for 2.4GHz - OK ...
02-14 11:41:42.341   568   568 D AT86RF215_Main at86rf215_setup_iq_radio_receive@at86rf215.c:769 Switching to receive - RX mode ...

```
## Output of the test_at86rf215_tx module

```
# Example output of:
./test_at86rf215_tx -f 433000000 -p 0x17 -s 400 -d 0_5 -a 80

# Options:
# --------
# -f --> supported frequency (in Hz), see at86rf215 datasheet for valid values
# -p --> TX gain - see at86rf215 datasheet for valid values
# -s --> sample rate - scaled value - 400 --> 400e3 = 400KSPS as minimum, 4000 --> 4000e3 = 4MSPS as maximum, see at86rf215 datasheet for valid values
# -d --> digital bandwidth, as string, "0_5" means 0.5 --> 0.5 *fS/2, see at86rf215 datasheet for valid values
# -a --> analog bandwidth - scaled value - 80 --> fLPFCUT = 80kHz, see at86rf215 datasheet for valid values

---------------------------- Sample Output of test_at86rf215_tx  ---------------------

02-14 12:06:01.712  1806  1806 D AT86RF215_Main at86rf215_init@at86rf215.c:168 Configuring reset and CS pins
02-14 12:06:01.714  1806  1806 D AT86RF215_Main at86rf215_init@at86rf215.c:179 Configuring SPI device modem
02-14 12:06:01.714  1806  1806 D AT86RF215_Main at86rf215_init@at86rf215.c:190 Configuring external interrupts
02-14 12:06:01.715  1806  1806 D AT86RF215_Main at86rf215_init@at86rf215.c:212 Modem identity: Version: 03, Product: 34
02-14 12:06:01.715  1806  1806 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:123 Calibration of modem channel 0...
02-14 12:06:01.786  1806  1806 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:139 Calibration Results of the modem: I=35, Q=25
02-14 12:06:01.786  1806  1806 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:123 Calibration of modem channel 1...
02-14 12:06:01.856  1806  1806 D AT86RF215_Main at86rf215_calibrate_device@at86rf215.c:139 Calibration Results of the modem: I=27, Q=29
TEST:AT86RF215:VERSIONS:PN=0x34
TEST:AT86RF215:VERSIONS:VN=3
TEST:AT86RF215:VERSIONS:PASS=1
TEST:AT86RF215:VERSIONS:INFO=The component PN is AT86RF215 (0x34), Version 3
-- AT86RF215 TX mode test performed --
at86rf215 RF frequency: 433000000l
at86rf215 TX power: 0x17
at86rf215 sample rate: 400
at86rf215 digital bandwidth: 0_5
at86rf215 analog bandwidth: 80
Applying configuration for sub-GHz ...
02-14 12:06:01.859  1806  1806 D AT86RF215_Main at86rf215_setup_iq_radio_transmit@at86rf215.c:638 Switching to TX preparation mode  ...
02-14 12:06:01.870  1806  1807 D AT86RF215_Events at86rf215_interrupt_handler@at86rf215_events.c:150 Hit callback - start ...
02-14 12:06:01.870  1806  1807 D AT86RF215_Events at86rf215_interrupt_handler@at86rf215_events.c:154 Hit callback - GPIO event rising edge
IRQ Status:
  Radio09:
    IQ_if_sync_fail: 0
    trx_error: 0
    battery_low: 0
    energy_detection_complete: 0
    trx_ready: 1
    wake_up_por: 0
  Radio24:
    IQ_if_sync_fail: 0
    trx_error: 0
    battery_low: 0
    energy_detection_complete: 0
    trx_ready: 0
    wake_up_por: 0
  Baseband09:
    frame_buffer_level: 0
    agc_release: 0
    agc_hold: 0
    frame_tx_complete: 0
    frame_rx_match_extended: 0
    frame_rx_address_match: 0
    frame_rx_complete: 0
    frame_rx_started: 0
  Baseband24:
    frame_buffer_level: 0
    agc_release: 0
    agc_hold: 0
    frame_tx_complete: 0
    frame_rx_match_extended: 0
    frame_rx_address_match: 0
    frame_rx_complete: 0
    frame_rx_started: 0
02-14 12:06:01.871  1806  1807 D AT86RF215_Events at86rf215_radio_event_handler@at86rf215_events.c:61 INT @ RADIO09: Transceiver ready
02-14 12:06:01.871  1806  1806 D AT86RF215_Main at86rf215_setup_iq_radio_transmit@at86rf215.c:651 Get hi_trx_ready_event for sub-GHz - OK ...
02-14 12:06:01.872  1806  1806 D AT86RF215_Main at86rf215_get_iq_sync_status@at86rf215.c:388 IQIFC2 - I/Q IF deserializer synchronization status: 1:

02-14 12:06:01.872  1806  1806 D AT86RF215_Main at86rf215_get_iq_sync_status@at86rf215.c:394 IQIFC0 - I/Q - no synchronization failure - OK ...
```
