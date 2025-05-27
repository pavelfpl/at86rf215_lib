#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "at86rf215.h"
#include "at86rf215_radio.h"
#include "io_utils/io_utils.h"

#define GPIO_CS_OFFSET 0
#define GPIO_RESET_OFFSET 1 
#define GPIO_EXT_INT_OFFSET 26 // 2 --> default on GPIOCHIP 2, 26 --> for GPIOCHIP 0

at86rf215_st dev =
{
    .cs_pin = GPIO_CS_OFFSET,
    .reset_pin = GPIO_RESET_OFFSET,
	.irq_pin = GPIO_EXT_INT_OFFSET,
    
    .spi_mode = 0,        // SPI mode (0)
    .spi_bits = 0,        // SPI bits mode (0 ... 8 bits) 
    .spi_speed = 1000000, // SPI baud rate
    
};

uint16_t unknown_regs[] = { 0x0015, 
                            0x0115, 0x0117, 0x0118, 0x0119, 0x011A, 0x011B, 0x011C, 0x011D, 0x011E, 0x011F, 0x0120, 0x0123, 0x0124, 0x0129,
                            0x0215, 0x0217, 0x0218, 0x0219, 0x021A, 0x021B, 0x021C, 0x021D, 0x021E, 0x021F, 0x0220, 0x0223, 0x0224, 0x0229};

uint8_t defaults[] =    {   0x0E,
                            0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0f, 0x0f, 0x08,
                            0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0f, 0x0f, 0x08};

// -----------------------------------------------------------------------------------------
                        
static void myflush ( FILE *in ){
  int ch;

  do
    ch = fgetc ( in ); 
  while ( ch != EOF && ch != '\n' ); 

  clearerr ( in );
}

static void mypause(void){
  
  myflush(stdin);
  printf ( "Press [Enter] to continue . . ." );
  fflush ( stdout );
  getchar();

}

static int str_compare(const char *input, char *output){

    int check_result = 1;

    for(int i=0; input[i]!='\0' || output[i]!='\0'; i++) {
        if(input[i] != output[i]) {
           check_result=0;
           break;
        }
    }
    return check_result;
}
                            
// This test checks the version number and part numbers of the device
int test_at86rf215_read_all_regs(at86rf215_st* dev)
{
    for (uint16_t reg = 0; reg < 0x300; reg ++)
    {
        uint8_t val = at86rf215_read_byte(dev, reg);
        printf("REG #0x%04X  :  0x%02X\n", reg, val);
    }
    return 0;
}

int test_at86rf215_read_all_regs_check(at86rf215_st* dev)
{
    int num_regs = sizeof(unknown_regs) / sizeof(uint16_t);

    for (int i = 0; i < num_regs; i ++)
    {
        uint8_t val = at86rf215_read_byte(dev, unknown_regs[i]);
        if (val != defaults[i]) printf("REG #0x%04X  :  0x%02X    (instead of 0x%02X)\n", unknown_regs[i], val, defaults[i]);
    }
    return 0;
}

// -----------------------------------------------------------------------------------------

// This test checks the version number and part numbers of the device
int test_at86rf215_read_chip_vn_pn(at86rf215_st* dev)
{
    int pass = 0;
    uint8_t pn, vn;
    char pn_st[15] = {0};
	at86rf215_get_versions(dev, &pn, &vn);

    if (pn==0x34 && vn > 0 && vn < 5) pass = 1;
    if (pn==0x34)
        sprintf(pn_st, "AT86RF215");
    else if (pn==0x35)
        sprintf(pn_st, "AT86RF215IQ");
    else if (pn==0x36)
        sprintf(pn_st, "AT86RF215M");
    else
        sprintf(pn_st, "UNKNOWN");

    printf("TEST:AT86RF215:VERSIONS:PN=0x%02X\n", pn);
    printf("TEST:AT86RF215:VERSIONS:VN=%d\n", vn);
    printf("TEST:AT86RF215:VERSIONS:PASS=%d\n", pass);
    printf("TEST:AT86RF215:VERSIONS:INFO=The component PN is %s (0x%02X), Version %d\n", pn_st, pn, vn);

    at86rf215_set_clock_output(dev, at86rf215_drive_current_8ma, at86rf215_clock_out_freq_32mhz);

    return pass;
}

// -----------------------------------------------------------------------------------------

// This test performs a frequency sweep on a certain channel:
//      at86rf215_rf_channel_900mhz / at86rf215_rf_channel_2400mhz
// and does it from "start_freq" to "start_freq + num_freq * step_freq"
// usec_gaps - specifies the micro-second gaps between freq steps or '-1' that
//             tell the function to put "getchars" (wait for enter key)
void test_at86rf215_sweep_frequencies(at86rf215_st* dev,
                                        at86rf215_rf_channel_en channel,
                                        int start_freq,
                                        int num_freq,
                                        int step_freq,
                                        int usec_gaps)
{
    char channel_name[10];

    if (channel == at86rf215_rf_channel_900mhz)
        sprintf(channel_name, "900MHz");
    else sprintf(channel_name, "2400MHz");

    io_utils_usleep(usec_gaps);
    int stop_freq = start_freq + step_freq*num_freq;
    int f = start_freq;
    while (f <= stop_freq)
    {
        printf("TEST:AT86RF215:FREQ_SWEEP:FREQ=%.2fMHz, CH=%s)\n", ((float)f) / 1e6, channel_name);
        at86rf215_setup_iq_radio_dac_value_override (dev,
                                                    channel,
                                                    f,
                                                    31);


        //printf("Press enter to switch\n");
        if (usec_gaps > 0) io_utils_usleep(usec_gaps);
        else
        {
            printf("Press enter to step...\n");
            getchar();
        }

        //test_at86rf215_read_all_regs_check(dev);
        f += step_freq;
    }

    // back to TX off state
    at86rf215_radio_set_state(dev, at86rf215_rf_channel_900mhz, at86rf215_radio_state_cmd_trx_off);
}

// -----------------------------------------------------------------------------------------
// Starting a reception window
// usec_timeout - set up a timeout value in micro-seconds or -1 to wait for "enter" key

int test_at86rf215_continues_iq_rx (at86rf215_st* dev, at86rf215_rf_channel_en radio,
                                        uint64_t freq_hz, int usec_timeout)
{

    at86rf215_rx_control_st rx_control ={0};
    rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW2000KHZ_IF2000KHZ;
    rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_5_half_fs;
    rx_control.fs = at86rf215_radio_rx_sample_rate_4000khz;
    rx_control.agc_enable = 1;
    rx_control.agc_gain = 0;
    rx_control.agc_relative_atten = at86rf215_radio_agc_relative_atten_21_db;
    rx_control.agc_averaging = at86rf215_radio_agc_averaging_8;

    at86rf215_iq_clock_data_skew_en skew = radio == at86rf215_rf_channel_900mhz?
                                            at86rf215_iq_clock_data_skew_4_906ns:
                                            at86rf215_iq_clock_data_skew_4_906ns;

    at86rf215_setup_iq_radio_receive (dev, radio, freq_hz,&rx_control, 0, skew);
    printf("Started I/Q RX session for Radio %d, Freq: %lu Hz, timeout: %d usec (0=infinity)\n",
        radio, freq_hz, usec_timeout);


    test_at86rf215_read_all_regs_check(dev);

    if (usec_timeout>0)
    {
        io_utils_usleep(usec_timeout);
    }
    else
    {
        printf("Press enter to stop...\n");
        getchar();
    }
    at86rf215_stop_iq_radio_receive (dev, radio);

    test_at86rf215_read_all_regs_check(dev);
    return 1;
}

// -----------------------------------------------------------------------------------------
// SMI communication tesing with loopback
// usec_timeout - set up a timeout value in micro-seconds or -1 to wait for "enter" key

int test_at86rf215_continues_iq_loopback (at86rf215_st* dev, at86rf215_rf_channel_en radio,
                                        uint32_t freq_hz, int usec_timeout)
{

    at86rf215_rx_control_st rx_control ={0};
    rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW2000KHZ_IF2000KHZ;
    rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_5_half_fs;
    rx_control.fs = at86rf215_radio_rx_sample_rate_4000khz;
    rx_control.agc_enable = 1;
    rx_control.agc_gain = 0;
    rx_control.agc_relative_atten = at86rf215_radio_agc_relative_atten_21_db;
    rx_control.agc_averaging = at86rf215_radio_agc_averaging_8;


    at86rf215_setup_iq_radio_receive (dev, radio, freq_hz, &rx_control, 0, at86rf215_iq_clock_data_skew_4_906ns);
    //at86rf215_radio_set_tx_dac_input_iq(dev, radio, 1, 0x7E, 1, 0x3F);

    printf("Started I/Q RX session for Radio %d, Freq: %d Hz, timeout: %d usec (0=infinity)\n",
        radio, freq_hz, usec_timeout);

    if (usec_timeout>0)
    {
        io_utils_usleep(usec_timeout);
    }
    else
    {
        printf("Press enter to stop...\n");
        getchar();
    }
    at86rf215_stop_iq_radio_receive (dev, radio);
    
    return 1;
}

// -----------------------------------------------------------------------------------------
// TEST SELECTION
// -----------------------------------------------------------------------------------------
#define TEST_VERSIONS       1
#define TEST_FREQ_SWEEP     0
#define TEST_IQ_RX_WIND     0
#define TEST_IQ_RX_WIND_RAD 0 
#define TEST_IQ_LB_WIND     0
#define TEST_READ_ALL_REGS  0

// -- Using CMAKE to define these MACROS --
// #define TEST_TX          1
// #define TEST_RX          1

// -----------------------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------------------
int main (int argc, char *argv[])
{
    int ret = 0;
    int opt = 0;
    char *end = NULL;
    
    at86rf215_iq_interface_config_st cfg = {0};
    at86rf215_irq_st irq = {0};

	if(at86rf215_init(&dev) == -1){
       return 1;   
    }
    
    // TEST: read the p/n and v/n from the IC
    #if TEST_VERSIONS
        test_at86rf215_read_chip_vn_pn(&dev);
    #endif // TEST_VERSIONS

    // TEST: Frequency sweep
    #if TEST_FREQ_SWEEP
        test_at86rf215_sweep_frequencies(&dev, at86rf215_rf_channel_900mhz, 900e6, 3, 10, 10000);
    #endif // TEST_FREQ_SWEEP

    #if TEST_IQ_RX_WIND
        #if TEST_IQ_RX_WIND_RAD==0
            test_at86rf215_continues_iq_rx (&dev, at86rf215_rf_channel_900mhz, 900e6, -1);
        #else
            test_at86rf215_continues_iq_rx (&dev, at86rf215_rf_channel_2400mhz, 2400e6, -1);
        #endif
    #endif // TEST_IQ_RX_WIND

    #if TEST_IQ_LB_WIND
        test_at86rf215_continues_iq_loopback (&dev, at86rf215_rf_channel_900mhz, 900e6, -1);
    #endif

    #if TEST_READ_ALL_REGS
        test_at86rf215_read_all_regs_check(&dev);
    #endif

    // -----------------------
    // -- TX testing option --
    // -----------------------
    #if TEST_TX==1
        at86rf215_tx_control_st tx_control ={0};

        unsigned long tx_freq = 0;
        int tx_power = 0;
        int sample_rate = 0;
        int analog_bw = 0;
        char digital_bw[10];

        printf("-- AT86RF215 TX mode test performed --\n");

        // -- Parse input arguments --
        while((opt = getopt(argc, argv, "f:p:s:a:d:")) != -1){
            switch(opt){
                case 'f':
                    tx_freq = strtoul(optarg, &end, 10);
                    printf("at86rf215 RF frequency: %ul\n", tx_freq);
                    if(optarg == end){
                       printf("Error parsing RF frequency - defaulting to 2.4GHz ...\n");
                       tx_freq = 2400e6;
                    }
                    break;
                case 'p':
                    tx_power = strtol(optarg, &end, 16);
                    printf("at86rf215 TX power: 0x%x\n", tx_power);
                    if(optarg == end){
                       printf("Error parsing TX power - defaulting to 0x0F ...\n");
                       tx_power = 0x0F;
                    }
                    break;
                case 's':
                    sample_rate = strtol(optarg, &end, 10);
                    printf("at86rf215 sample rate: %d\n",sample_rate);
                    if(optarg == end){
                       printf("Error parsing sample rate - defaulting to 4Msps ...\n");
                       sample_rate = 4000;
                    }
                    break;
                case 'a':
                    analog_bw = strtol(optarg, &end, 10);
                    printf("at86rf215 analog bandwidth: %d\n", analog_bw);
                    if(optarg == end){
                       printf("Error parsing analog bw - defaulting to 1000khz ...\n");
                       analog_bw = 1000;
                    }
                    break;
                case 'd':
                    if(optarg != NULL){
                       strcpy(digital_bw, optarg);
                       printf("at86rf215 digital bandwidth: %s\n",digital_bw);
                    }else{
                       strcpy(digital_bw, "0_5");
                    }

                    break;
                default:
                    break;
            }
        }

        tx_control.tx_control_with_iq_if = 0;
        tx_control.pa_ramping_time = at86rf215_radio_tx_pa_ramp_4usec;
        tx_control.current_reduction = at86rf215_radio_pa_current_reduction_0ma;
        tx_control.tx_power = tx_power;                 // Max power: 0x1F, 0x10

        // -- Parse analog bw --
        switch(analog_bw){
            case 1000:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_1000khz;
                 break;
            case 800:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_800khz;
                 break;
            case 625:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_625khz;
                 break;
            case 500:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_500khz;
                 break;
            case 400:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_400khz;
                 break;
            case 315:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_315khz;
                 break;
            case 250:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_250khz;
                 break;
            case 200:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_200khz;
                 break;
            case 160:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_160khz;
                 break;
            case 125:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_125khz;
                 break;
            case 100:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_100khz;
                 break;
            case 80:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_80khz;
                 break;
            default:
                 tx_control.analog_bw = at86rf215_radio_tx_cut_off_1000khz;
                 break;
        }

        // -- Parse digital bw --
        if(str_compare("0_25", digital_bw)){
           tx_control.digital_bw = at86rf215_radio_rx_f_cut_0_25_half_fs;
        }else if(str_compare("0_375", digital_bw)){
           tx_control.digital_bw = at86rf215_radio_rx_f_cut_0_375_half_fs;
        }else if(str_compare("0_5", digital_bw)){
           tx_control.digital_bw = at86rf215_radio_rx_f_cut_0_5_half_fs;
        }else if(str_compare("0_75", digital_bw)){
           tx_control.digital_bw = at86rf215_radio_rx_f_cut_0_75_half_fs;
        }else if(str_compare("1_00", digital_bw)){
           tx_control.digital_bw = at86rf215_radio_rx_f_cut_half_fs;
        }else{
           tx_control.digital_bw = at86rf215_radio_rx_f_cut_0_5_half_fs;
        }

        // -- Parse sample rate - e.g: at86rf215_radio_rx_sample_rate_4000khz --
        switch(sample_rate){
            case 4000:
                tx_control.fs = at86rf215_radio_rx_sample_rate_4000khz;
                break;
            case 2000:
                tx_control.fs = at86rf215_radio_rx_sample_rate_2000khz;
                break;
            case 1333:
                tx_control.fs = at86rf215_radio_rx_sample_rate_1333khz;
                break;
            case 1000:
                tx_control.fs = at86rf215_radio_rx_sample_rate_1000khz;
                break;
            case 800:
                tx_control.fs = at86rf215_radio_rx_sample_rate_800khz;
                break;
            case 666:
                tx_control.fs = at86rf215_radio_rx_sample_rate_666khz;
                break;
            case 500:
                tx_control.fs = at86rf215_radio_rx_sample_rate_500khz;
                break;
            case 400:
                tx_control.fs = at86rf215_radio_rx_sample_rate_400khz;
                break;
            default:
                tx_control.fs = at86rf215_radio_rx_sample_rate_4000khz;
                break;
        }
        
        // -- Set radio --
        if(tx_freq < 1000e6){
           printf("Applying configuration for sub-GHz ...\n");
           at86rf215_setup_iq_radio_transmit (&dev, at86rf215_rf_channel_900mhz, tx_freq, &tx_control, 0, at86rf215_iq_clock_data_skew_1_906ns);  // 433e6
        }else{
           printf("Applying configuration for 2.4GHz ... \n");
           at86rf215_setup_iq_radio_transmit (&dev, at86rf215_rf_channel_2400mhz, tx_freq, &tx_control, 0, at86rf215_iq_clock_data_skew_1_906ns); // 2400e6
        }

        // Wait for user input ...
        mypause();
        
        // Print status at the end ...
        at86rf215_get_iq_sync_status(&dev);
        at86rf215_get_irqs(&dev, &irq, 1);
    #endif

    // -----------------------
    // -- RX testing option --
    // -----------------------
    #if TEST_RX==1
        printf("-- AT86RF215 RX mode test performed --\n");

        at86rf215_rx_control_st rx_control ={0};

        unsigned long rx_freq = 0;
        int agc_enable = 0;
        int rx_gain = 0;
        int sample_rate = 0;
        int analog_bw = 0;
        char digital_bw[10];

        // -- Parse input arguments --
        while((opt = getopt(argc, argv, "f:e:g:s:a:d:")) != -1){
            switch(opt){
                case 'f':
                    rx_freq = strtoul(optarg, &end, 10);
                    printf("at86rf215 RF frequency: %ul\n", rx_freq);
                    if(optarg == end){
                       printf("Error parsing RF frequency - defaulting to 2.4GHz ...\n");
                       rx_freq = 2400e6;
                    }
                    break;
                case 'e':
                    agc_enable = strtol(optarg, &end, 10);
                    printf("at86rf215 AGC enable: %d\n", agc_enable);
                    if(optarg == end){
                       printf("AGC enable parsing error - defaulting to 0 ...\n");
                       agc_enable = 0;
                    }
                case 'g':
                    rx_gain = strtol(optarg, &end, 16);
                    printf("at86rf215 RX gain: 0x%x\n", rx_gain);
                    if(optarg == end){
                       printf("Error parsing RX gain - defaulting to 0 ...\n");
                       rx_gain = 0x0;
                    }
                    break;
                case 's':
                    sample_rate = strtol(optarg, &end, 10);
                    printf("at86rf215 sample rate: %d\n",sample_rate);
                    if(optarg == end){
                       printf("Error parsing sample rate - defaulting to 4Msps ...\n");
                       sample_rate = 4000;
                    }
                    break;
                case 'a':
                    analog_bw = strtol(optarg, &end, 10);
                    printf("at86rf215 analog bandwidth: %d\n", analog_bw);
                    if(optarg == end){
                       printf("Error parsing analog bw - defaulting to 1000khz ...\n");
                       analog_bw = 1000;
                    }
                    break;
                case 'd':
                    if(optarg != NULL){
                       strcpy(digital_bw, optarg);
                       printf("at86rf215 digital bandwidth: %s\n",digital_bw);
                    }else{
                       strcpy(digital_bw, "0_5");
                    }

                    break;
                default:
                    break;
            }
        }

        // -- Parse analog bw --
        switch(analog_bw){
            case 2000:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW2000KHZ_IF2000KHZ;
                 break;
            case 1600:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW1600KHZ_IF2000KHZ;
                 break;
            case 1250:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW1250KHZ_IF2000KHZ;
                 break;
            case 1000:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW1000KHZ_IF1000KHZ;
                 break;
            case 800:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW800KHZ_IF1000KHZ;
                 break;
            case 630:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW630KHZ_IF1000KHZ;
                 break;
            case 500:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW500KHZ_IF500KHZ;
                 break;
            case 400:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW400KHZ_IF500KHZ;
                 break;
            case 320:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW320KHZ_IF500KHZ;
                 break;
            case 250:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW250KHZ_IF250KHZ;
                 break;
            case 200:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW200KHZ_IF250KHZ;
                 break;
            case 160:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW160KHZ_IF250KHZ;
                 break;
            default:
                 rx_control.radio_rx_bw = at86rf215_radio_rx_bw_BW2000KHZ_IF2000KHZ;
                 break;
        }

        // -- Parse digital bw --
        if(str_compare("0_25", digital_bw)){
           rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_25_half_fs;
        }else if(str_compare("0_375", digital_bw)){
           rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_375_half_fs;
        }else if(str_compare("0_5", digital_bw)){
           rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_5_half_fs;
        }else if(str_compare("0_75", digital_bw)){
           rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_75_half_fs;
        }else if(str_compare("1_00", digital_bw)){
           rx_control.digital_bw = at86rf215_radio_rx_f_cut_half_fs;
        }else{
           rx_control.digital_bw = at86rf215_radio_rx_f_cut_0_5_half_fs;
        }

        // -- Parse sample rate - e.g: at86rf215_radio_rx_sample_rate_4000khz --
        switch(sample_rate){
            case 4000:
                rx_control.fs = at86rf215_radio_rx_sample_rate_4000khz;
                break;
            case 2000:
                rx_control.fs = at86rf215_radio_rx_sample_rate_2000khz;
                break;
            case 1333:
                rx_control.fs = at86rf215_radio_rx_sample_rate_1333khz;
                break;
            case 1000:
                rx_control.fs = at86rf215_radio_rx_sample_rate_1000khz;
                break;
            case 800:
                rx_control.fs = at86rf215_radio_rx_sample_rate_800khz;
                break;
            case 666:
                rx_control.fs = at86rf215_radio_rx_sample_rate_666khz;
                break;
            case 500:
                rx_control.fs = at86rf215_radio_rx_sample_rate_500khz;
                break;
            case 400:
                rx_control.fs = at86rf215_radio_rx_sample_rate_400khz;
                break;
            default:
                rx_control.fs = at86rf215_radio_rx_sample_rate_4000khz;
                break;
        }

        rx_control.agc_enable = agc_enable;
        rx_control.agc_gain = rx_gain;
        rx_control.agc_relative_atten = at86rf215_radio_agc_relative_atten_21_db;
        rx_control.agc_averaging = at86rf215_radio_agc_averaging_8;

        // -- Set radio --
        if(rx_freq < 1000e6){
           printf("Applying configuration for sub-GHz ...\n");
           at86rf215_setup_iq_radio_receive (&dev, at86rf215_rf_channel_900mhz, rx_freq, &rx_control, 0, at86rf215_iq_clock_data_skew_1_906ns);  // 433e6
        }else{
           printf("Applying configuration for 2.4GHz ... \n");
           at86rf215_setup_iq_radio_receive (&dev, at86rf215_rf_channel_2400mhz, rx_freq, &rx_control, 0, at86rf215_iq_clock_data_skew_1_906ns); // 2400e6
        }

        // Wait for user input ...
        mypause();

        // -- Disabling RX reception --
        if(rx_freq < 1000e6){
           printf("Stopping RX reception on sub-GHz radio ...\n");
           at86rf215_stop_iq_radio_receive (&dev, at86rf215_rf_channel_900mhz);
        }else{
           printf("Stopping RX reception on 2.4GHz radio ...\n");
           at86rf215_stop_iq_radio_receive (&dev, at86rf215_rf_channel_2400mhz);
        }

        // Print status at the end ...
        at86rf215_get_iq_sync_status(&dev);
        at86rf215_get_irqs(&dev, &irq, 1);
    #endif

    // Close and reset device if requested ... 
    at86rf215_close(&dev, 1); // Reset dev 1, no reset 0 ...
    
    return 0;
}
