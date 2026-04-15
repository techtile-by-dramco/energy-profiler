/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: main.c
 *      Created: 2024-03-20
 *       Author: Jarne Van Mulders
 *      Version: major.minor
 *
 *  Description: END BLE monitor board
 * 
 *  Commissiond by Reindeer POC and demonstrator
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include "stdio.h"
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>

#define CALIBRATION
#define AUTO_RESISTANCE_TUNE
#define BROADCAST_DATA

//  Default setpoint and accuracy
#define SETPOINT    1800    //  mV
#define SP_ACCURACY  50      //  mV

#define SAMPLE_FREQ     5 //250
#define TIMER_PERIOD    1000/SAMPLE_FREQ //2

//  Include external I2C peripherals
#include "peripherals/peripherals.h"

//  Timer settings LED blink
uint16_t delay_on_ms = 500;
uint16_t delay_off_ms = 500;
struct k_timer my_timer;

//  Potentiometer settings
int32_t pot_val = 0;
#define LOWER_BOUNDARY 0 
#define UPPER_BOUNDARY 719//487 // Lowest resistance
uint32_t target_voltage = SETPOINT;

//  Variables for UART
int32_t buffer_voltage_mv;
uint32_t resistance;
uint32_t pwr_pw;

//  GPIO pins
struct gpio_dt_spec blue_led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(usrled0), gpios);
struct gpio_dt_spec red_led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(usrled1), gpios);
struct gpio_dt_spec pot100k_pin = GPIO_DT_SPEC_GET(DT_ALIAS(pot100k), gpios);
struct gpio_dt_spec pot10k_pin = GPIO_DT_SPEC_GET(DT_ALIAS(pot10k), gpios);

//  UART device
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

//  Static functions
static uint8_t calculate_xor_checksum(const uint8_t* buffer, size_t length);
static bool parse_frame(const uint8_t *buf, size_t len, uint8_t *cmd, uint32_t *value);
static int debugger();
static uint8_t rx_buf[1000] = { 0 };

//  Stack size and debug priority
#define STACKSIZE 1024
#define DBG_PRIORITY 8

//  *** START THREADS *** //
K_THREAD_DEFINE(debugger_id, STACKSIZE, debugger, NULL, NULL, NULL, DBG_PRIORITY, 0, 0);

//  P controller gains
#define KP 0.02     // Proportional gain

//  Upper and lower setpoint limits
uint32_t upper_set_point = 0;
uint32_t lower_set_point = 0;

//  Other global variables
uint32_t num = 0;
uint32_t max_voltage = 0;
uint8_t timer_elapsed = 0;

// Define uint8_t transmit array to store converted values
uint8_t buf8 [19]; // 1 (start) + 1 (length) + 4*4 (data) + 1 (checksum)
uint32_t buf32 [4]; 

//  Lookup table for resistance values corresponding to potentiometer settings
const float lookup_table[719] = { 959538.00f, 956019.00f, 952684.00f, 949355.00f, 946061.00f, 942132.00f, 938460.00f, 935074.00f, 931683.00f, 928149.00f, 924660.00f, 921208.00f, 917833.00f, 914309.00f, 910815.00f, 907424.00f, 904003.00f, 900504.00f, 897026.00f, 893558.00f, 889978.00f, 886664.00f, 883196.00f, 879488.00f, 876056.00f, 872537.00f, 868997.00f, 865422.00f, 861888.00f, 858446.00f, 855009.00f, 851495.00f, 847935.00f, 844334.00f, 840774.00f, 837322.00f, 833834.00f, 830294.00f, 826719.00f, 823102.00f, 819553.00f, 816044.00f, 812566.00f, 809022.00f, 805328.00f, 801661.00f, 798229.00f, 794900.00f, 791611.00f, 788205.00f, 784763.00f, 781008.00f, 776967.00f, 773289.00f, 769775.00f, 766277.00f, 762758.00f, 759080.00f, 755433.00f, 751847.00f, 748277.00f, 744712.00f, 741178.00f, 737521.00f, 733910.00f, 730303.00f, 726764.00f, 723204.00f, 719583.00f, 715982.00f, 712350.00f, 708898.00f, 705138.00f, 701522.00f, 697875.00f, 694253.00f, 690576.00f, 686934.00f, 683379.00f, 679839.00f, 676085.00f, 672310.00f, 668765.00f, 665205.00f, 661569.00f, 657896.00f, 654249.00f, 650628.00f, 646945.00f, 643318.00f, 639661.00f, 635998.00f, 632351.00f, 628694.00f, 625022.00f, 621380.00f, 617666.00f, 613983.00f, 610249.00f, 606633.00f, 603037.00f, 599272.00f, 595297.00f, 591615.00f, 588065.00f, 584582.00f, 580996.00f, 577190.00f, 573431.00f, 569779.00f, 566142.00f, 562418.00f, 558669.00f, 554965.00f, 551282.00f, 547543.00f, 543870.00f, 540162.00f, 536479.00f, 532740.00f, 528975.00f, 525190.00f, 521548.00f, 517757.00f, 514054.00f, 510351.00f, 506555.00f, 502877.00f, 499066.00f, 495297.00f, 491603.00f, 487869.00f, 484084.00f, 480401.00f, 476805.00f, 473076.00f, 468953.00f, 465198.00f, 461516.00f, 457694.00f, 453904.00f, 450144.00f, 446400.00f, 442620.00f, 438829.00f, 435105.00f, 431269.00f, 427525.00f, 423765.00f, 419980.00f, 416169.00f, 412378.00f, 408542.00f, 404762.00f, 400956.00f, 397047.00f, 393365.00f, 389656.00f, 385779.00f, 381937.00f, 378085.00f, 374233.00f, 370427.00f, 366606.00f, 362805.00f, 358984.00f, 355153.00f, 351337.00f, 347470.00f, 343654.00f, 339797.00f, 335781.00f, 332006.00f, 328220.00f, 324368.00f, 320547.00f, 316670.00f, 312833.00f, 308961.00f, 305124.00f, 301242.00f, 297390.00f, 293599.00f, 289645.00f, 285773.00f, 281885.00f, 278002.00f, 274104.00f, 270232.00f, 266380.00f, 262426.00f, 258610.00f, 254614.00f, 250737.00f, 246828.00f, 242936.00f, 239043.00f, 235160.00f, 231206.00f, 227282.00f, 223374.00f, 219455.00f, 215501.00f, 211629.00f, 207710.00f, 203802.00f, 199889.00f, 195944.00f, 191995.00f, 188056.00f, 184117.00f, 180194.00f, 176234.00f, 172295.00f, 168320.00f, 164381.00f, 160422.00f, 156478.00f, 152503.00f, 148554.00f, 144605.00f, 140655.00f, 136675.00f, 132716.00f, 128598.00f, 124623.00f, 120648.00f, 116668.00f, 112688.00f, 108698.00f, 104697.00f, 100712.00f, 87181.50f, 86873.80f, 86565.50f, 86257.80f, 85950.60f, 85643.30f, 85335.10f, 85026.80f, 84716.50f, 84406.10f, 84096.80f, 83787.50f, 83478.20f, 83168.30f, 82859.00f, 82546.50f, 82207.80f, 81898.50f, 81588.70f, 81280.40f, 80970.60f, 80660.70f, 80348.80f, 80033.70f, 79717.00f, 79401.90f, 79089.00f, 78776.00f, 78464.00f, 78150.00f, 77835.40f, 77520.90f, 77171.10f, 76857.10f, 76542.00f, 76226.90f, 75912.80f, 75596.70f, 75281.10f, 74965.00f, 74648.80f, 74333.20f, 74015.50f, 73697.20f, 73381.10f, 73062.30f, 72746.20f, 72426.90f, 72095.50f, 71776.20f, 71457.50f, 71137.10f, 70818.90f, 70499.60f, 70180.80f, 69860.50f, 69541.70f, 69218.80f, 68896.80f, 68577.00f, 68256.70f, 67935.80f, 67613.90f, 67291.50f, 66951.20f, 66628.20f, 66305.20f, 65982.80f, 65659.30f, 65336.30f, 65012.80f, 64689.30f, 64365.30f, 64040.30f, 63716.80f, 63389.60f, 63066.10f, 62740.50f, 62414.90f, 62089.30f, 61748.50f, 61420.80f, 61091.50f, 60763.80f, 60437.70f, 60111.60f, 59787.60f, 59459.90f, 59130.60f, 58801.40f, 58472.60f, 58144.40f, 57815.10f, 57484.80f, 57156.10f, 56825.70f, 56477.00f, 56147.30f, 55816.90f, 55485.00f, 55154.20f, 54822.30f, 54490.90f, 54159.00f, 53826.60f, 53494.70f, 53161.80f, 52827.80f, 52495.90f, 52161.90f, 51828.40f, 51494.40f, 51151.50f, 50817.00f, 50482.50f, 50146.90f, 49812.40f, 49475.70f, 49141.80f, 48807.80f, 48472.70f, 48131.40f, 47793.20f, 47457.10f, 47120.40f, 46781.70f, 46444.10f, 46105.90f, 45750.30f, 45412.10f, 45073.40f, 44733.60f, 44394.90f, 44053.60f, 43715.90f, 43375.10f, 43034.80f, 42693.40f, 42353.10f, 42011.30f, 41669.90f, 41328.00f, 40985.60f, 40641.70f, 40292.40f, 39950.60f, 39607.10f, 39262.10f, 38919.70f, 38575.70f, 38231.80f, 37887.30f, 37542.80f, 37197.70f, 36852.70f, 36506.60f, 36161.60f, 35814.00f, 35467.90f, 35121.80f, 34758.40f, 34410.80f, 34063.60f, 33716.00f, 33368.30f, 33019.60f, 32672.00f, 32322.80f, 31974.10f, 31623.80f, 31275.10f, 30924.30f, 30575.10f, 30225.30f, 29873.50f, 29521.60f, 29162.90f, 28812.10f, 28460.30f, 28107.40f, 27754.50f, 27402.10f, 27049.70f, 26696.30f, 26343.90f, 25988.90f, 25635.00f, 25280.50f, 24926.00f, 24570.50f, 24216.00f, 23859.50f, 23483.40f, 23126.90f, 22770.30f, 22413.20f, 22056.10f, 21699.50f, 21342.40f, 20984.30f, 20627.20f, 20268.00f, 19908.80f, 19550.10f, 19190.90f, 18831.10f, 18471.90f, 18111.20f, 17743.60f, 17382.80f, 17021.50f, 16659.60f, 16297.80f, 15936.50f, 15574.70f, 15212.30f, 14850.00f, 14486.60f, 14123.20f, 13759.80f, 13395.80f, 13031.40f, 12667.50f, 12301.40f, 11924.90f, 11559.90f, 11193.90f, 10828.40f, 10462.40f, 10094.70f, 9728.70f, 9361.60f, 9670.40f, 9635.20f, 9599.00f, 9562.20f, 9527.10f, 9489.30f, 9454.10f, 9416.80f, 9381.10f, 9344.80f, 9308.60f, 9271.80f, 9236.10f, 9163.90f, 9128.29f, 9091.41f, 9041.42f, 9005.60f, 8969.77f, 8933.06f, 8897.44f, 8860.57f, 8824.85f, 8788.13f, 8752.46f, 8715.54f, 8679.87f, 8643.20f, 8607.64f, 8570.71f, 8535.15f, 8498.22f, 8445.98f, 8410.05f, 8374.22f, 8337.45f, 8301.79f, 8264.86f, 8229.14f, 8192.32f, 8156.65f, 8119.72f, 8084.05f, 8047.29f, 8011.62f, 7974.74f, 7938.86f, 7902.04f, 7851.90f, 7815.97f, 7780.09f, 7743.22f, 7707.39f, 7670.52f, 7634.74f, 7598.03f, 7562.20f, 7525.17f, 7489.55f, 7452.57f, 7416.96f, 7379.98f, 7344.21f, 7307.33f, 7254.98f, 7219.00f, 7183.02f, 7146.20f, 7110.37f, 7073.44f, 7037.62f, 7000.69f, 6964.97f, 6927.94f, 6892.06f, 6855.19f, 6819.47f, 6782.54f, 6746.66f, 6709.79f, 6659.43f, 6623.35f, 6587.42f, 6550.59f, 6514.66f, 6477.63f, 6441.75f, 6404.93f, 6368.95f, 6331.97f, 6296.09f, 6259.22f, 6223.39f, 6186.41f, 6150.54f, 6113.50f, 6061.05f, 6024.91f, 5988.98f, 5952.05f, 5916.07f, 5879.04f, 5843.11f, 5806.18f, 5770.25f, 5733.22f, 5697.29f, 5660.26f, 5624.48f, 5587.45f, 5551.47f, 5514.49f, 5463.98f, 5427.79f, 5391.80f, 5354.77f, 5318.79f, 5281.70f, 5245.72f, 5208.74f, 5172.76f, 5135.68f, 5099.75f, 5062.71f, 5026.84f, 4989.70f, 4953.72f, 4916.69f, 4864.28f, 4828.09f, 4791.95f, 4754.92f, 4718.88f, 4681.70f, 4645.77f, 4608.68f, 4572.70f, 4535.61f, 4499.58f, 4462.49f, 4426.46f, 4389.38f, 4353.39f, 4316.31f, 4265.80f, 4229.55f, 4193.36f, 4156.33f, 4120.19f, 4083.05f, 4047.02f, 4009.88f, 3973.79f, 3936.65f, 3900.62f, 3863.48f, 3827.50f, 3790.42f, 3754.28f, 3717.14f, 3664.42f, 3628.18f, 3591.99f, 3554.85f, 3518.71f, 3481.52f, 3445.38f, 3408.30f, 3372.16f, 3334.97f, 3298.88f, 3261.69f, 3225.66f, 3188.47f, 3152.27f, 3115.19f, 3064.31f, 3028.07f, 2991.77f, 2954.58f, 2918.39f, 2881.14f, 2844.95f, 2807.82f, 2771.62f, 2734.38f, 2698.29f, 2661.05f, 2624.96f, 2587.72f, 2551.53f, 2514.29f, 2461.57f, 2425.28f, 2389.03f, 2351.74f, 2315.54f, 2278.25f, 2242.06f, 2204.82f, 2168.62f, 2131.33f, 2095.14f, 2057.84f, 2021.70f, 1984.41f, 1948.16f, 1910.98f, 1860.10f, 1823.69f, 1787.40f, 1750.10f, 1713.80f, 1676.51f, 1640.27f, 1602.97f, 1566.73f, 1529.28f, 1493.14f, 1455.74f, 1419.65f, 1382.25f, 1346.06f, 1308.66f, 1255.84f, 1219.38f, 1182.98f, 1145.63f, 1109.34f, 1071.99f, 1035.64f, 998.29f, 962.05f, 924.65f, 893.94f, 856.16f, 819.60f, 781.80f, 745.14f, 707.43f, 653.43f, 616.65f, 579.94f, 542.31f, 505.69f, 468.02f, 431.45f, 393.85f, 357.30f, 319.63f, 283.14f, 245.54f, 209.12f, 171.48f, 134.96f };

// Timer callback function
void my_timer_handler(struct k_timer *dummy) {
    timer_elapsed = 1;
}

bool new_uart_message = false;

const uint8_t *buf;
size_t len;

//  UART callback function
static void uart_cb(const struct device* dev, struct uart_event* evt, void* user_data)
{
	switch (evt->type) {

    case UART_RX_RDY:
        //  Get pointer to received data and its length
        buf = evt->data.rx.buf + evt->data.rx.offset;
        len = evt->data.rx.len;

        new_uart_message = true;

        break;

	default:
		break;
	}
}

//  Function to parse incoming UART frames
bool parse_frame(const uint8_t *buf, size_t len, uint8_t *cmd, uint32_t *value)
{
    if (len < 7) {
        return false;  // minimaal frame past niet
    }

    // Doorloop de buffer om een geldig frame te vinden
    for (size_t i = 0; i <= len - 7; i++) {
        if (buf[i] != 0x02) {
            continue;  // niet het begin, verder zoeken
        }

        uint8_t data_len = buf[i + 2];
        if (data_len != 4) {
            continue;  // ongeldig data_len, verder zoeken
        }

        if (i + 3 + data_len >= len) {
            continue;  // buffer te klein voor volledige data + end delimiter
        }

        // if (buf[i + 3 + data_len] != 0xFF) {
        //     continue;  // geen correct einde, verder zoeken
        // }

        // Frame geldig → data uitlezen
        *cmd = buf[i + 1];
        *value = ((uint32_t)buf[i + 3] << 24) |
                 ((uint32_t)buf[i + 4] << 16) |
                 ((uint32_t)buf[i + 5] << 8)  |
                 ((uint32_t)buf[i + 6]);

        return true;  // frame gevonden
    }

    return false;  // geen geldig frame gevonden
}

//  Function to send telemetry data over UART
void on_timer_event(){
    // Reset timer elapsed variable
    timer_elapsed = 0;

    // Update uint32_t array
    buf32 [0] = buffer_voltage_mv;
    buf32 [1] = resistance;
    buf32 [2] = pwr_pw;
    buf32 [3] = pot_val;

    // Calculate total number of elements in buf32 array
    uint8_t buf32_size = sizeof(buf32) / sizeof(buf32[0]);

    // Begin delimter + length
    buf8[0] = 0x02;
    buf8[1] = buf32_size * sizeof(uint32_t) + 1;

    // Perform conversion
    size_t idx = 2;
    for (uint8_t i = 0; i < 4; i++) {
        // Extract each 32-bit value into 4 bytes
        buf8[idx++] = (uint8_t)(buf32[i] >> 24);  // Most significant byte (MSB)
        buf8[idx++] = (uint8_t)(buf32[i] >> 16);
        buf8[idx++] = (uint8_t)(buf32[i] >> 8);
        buf8[idx++] = (uint8_t)(buf32[i]);        // Least significant byte (LSB)
    }

    buf8[18] = calculate_xor_checksum(buf8, 18);

    uart_tx(uart, buf8, sizeof(buf8), SYS_FOREVER_US);
}

// Function to calculate XOR checksum
uint8_t calculate_xor_checksum(const uint8_t* buffer, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum ^= buffer[i];
    }
    return checksum;
}


int main(void)
{
    //  Configure GPIO pins
    gpio_pin_configure_dt(&blue_led_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&red_led_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&pot100k_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&pot10k_pin, GPIO_OUTPUT_INACTIVE);

    // Init peripherals (ADS and programmable load)
    perihperals_init();

    // Set initial potentiometer value to max resistance
    peripherals_set_digital_potentiometer(pot_val);

	//  Check if UART device is ready
	if (!device_is_ready(uart)) {
		printk("UART device not ready\n\r");
		return 0;
	}

    //  Set UART callback
	uart_callback_set(uart, uart_cb, NULL);
	uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 10000);

    // Initialize the timer with the handler function and set interval
    k_timer_init(&my_timer, my_timer_handler, NULL);

    /* start a periodic timer that expires once every second */
    k_timer_start(&my_timer, K_MSEC(TIMER_PERIOD), K_MSEC(TIMER_PERIOD));

    //  Initialize limits
    upper_set_point = target_voltage + SP_ACCURACY;
    lower_set_point = target_voltage - SP_ACCURACY;

	while (1) {
		k_msleep(1);


        if(new_uart_message){

            uart_rx_disable(uart);

            
            uart_tx(uart, buf, len, SYS_FOREVER_US);

            //  Local variables to store parsed command and value
            uint8_t cmd;
            uint32_t value;

            //  Parse the received frame
            if (parse_frame(buf, len, &cmd, &value)) {
                printk("CMD = 0x%02X\n", cmd);
                printk("VALUE = %u (0x%08X)\n", value, value);

                value = (int32_t)value;

                //  Check command and execute corresponding action
                switch (cmd)
                {
                case 1:
                    //  Limit potentiometer value to upper boundary
                    if(value > UPPER_BOUNDARY){
                        value = UPPER_BOUNDARY;
                    }

                    pot_val = value;
                    peripherals_set_digital_potentiometer(value);
                    break;
                    
                case 2:
                    target_voltage = value;

                    //  Update limits
                    upper_set_point = target_voltage + SP_ACCURACY;
                    lower_set_point = target_voltage - SP_ACCURACY;
                    break;    
                
                default:
                    break;
                }


                uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 10000);
                
            } else {
                printk("Invalid frame\n");
            }

            new_uart_message = false;
        }



        // Timer event handling
        if(timer_elapsed == 1){ // UPDATE --> DIT GEBRUIKEN
            #ifdef BROADCAST_DATA
                on_timer_event();
            #endif
        }


        // k_usleep(500); // UPDATE --> DIT WEG

        // // peripherals_ads1115_read_one_channel(0);
        // // buffer_voltage_mv = peripherals_ads1115_read(ADS1115_COMP_0_GND);

        int32_t voltage = peripherals_ads1115_read(ADS1115_COMP_0_GND);

        if (voltage > 0) {
            buffer_voltage_mv = voltage;
        }
        else {
            buffer_voltage_mv = 0;
        }

        //  Calculate resistance value
        #ifndef CALIBRATION
            if(pot_val < 232){
                resistance = (256 - pot_val) * 1e6 / 256;
            }
            else if(pot_val >= 232 && pot_val < 464){
                resistance = (256 - (pot_val - 232)) * 1e5 / 256; // 94300 is the adapted value for 100k potentiometer
                resistance = (resistance * 1e6)/(resistance + 1e6);
            }
            else{
                resistance = (256 - (pot_val - 464)) * 1e4 / 256; // 94300 is the adapted value for 100k potentiometer
                resistance = (resistance * 1e6)/(resistance + 1e6);
            }
        #else
            resistance = lookup_table[(uint16_t)pot_val];
        #endif

        //  Calculate pwr level
        pwr_pw = buffer_voltage_mv*buffer_voltage_mv*1e6 / resistance;


        #ifdef AUTO_RESISTANCE_TUNE
            if(buffer_voltage_mv > 2000){
                num++;
            }

            if(max_voltage < buffer_voltage_mv){
                max_voltage = buffer_voltage_mv;
            }

            //  Reset performance measurements
            if(pot_val == 0) { num = 0; max_voltage = 0; }

            //  First buffer_voltage check
            if(buffer_voltage_mv > upper_set_point){
                uint16_t error = (buffer_voltage_mv - upper_set_point)*KP;
                if(!error){error = 1;}
                pot_val = pot_val + error;
                if (pot_val < UPPER_BOUNDARY) {
                    peripherals_set_digital_potentiometer(pot_val);
                }
                else{
                    pot_val = UPPER_BOUNDARY;
                    peripherals_set_digital_potentiometer(UPPER_BOUNDARY);
                }
            }

            //  Second buffer_voltage check
            if(buffer_voltage_mv < lower_set_point){
                uint16_t error = (lower_set_point - buffer_voltage_mv)*KP;
                if(!error){error = 1;}
                pot_val = pot_val - error;
                if (pot_val > LOWER_BOUNDARY) {
                    peripherals_set_digital_potentiometer(pot_val);
                }
                else{
                    pot_val = LOWER_BOUNDARY;
                    peripherals_set_digital_potentiometer(LOWER_BOUNDARY);
                }
            }
        #endif
	}

    return 0;
}

// #define PRINT_DEBUG
#define BLINK_LED


int debugger(){

    #ifdef PRINT_DEBUG

    while(1){

        k_msleep(500);

        printk("Buffer voltage: %d mV\n", buffer_voltage_mv);

        printk("Pot value: %d\n", pot_val);

        printk("resistance: %d\n", resistance);

        printk("Power: %d nW\n", pwr_pw);

        // printk("%d\n", num);

        printk("Approximated time above 2V: %d ms\n", num*947/1000); // loop speed (947 us) measured with logic analyser

        printk("Max. voltage measured: %d mV\n", max_voltage);

        // return 1;

    }

    #endif

    #ifdef BLINK_LED

    uint64_t curr_time = 0;
    uint64_t prev_time = k_uptime_get();

    uint8_t state = 0;

    while(1){
        //  Update current time
        curr_time = k_uptime_get();

        //  Blink LED
        if(curr_time - prev_time > 50){
            prev_time = curr_time;
            if(state)   {   gpio_pin_set_dt(&blue_led_pin, 1);  state = 0;  }
            else        {   gpio_pin_set_dt(&blue_led_pin, 0);  state = 1;  }
        }
    }
    #endif

    return 0;
}