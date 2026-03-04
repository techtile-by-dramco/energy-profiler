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
#define UPPER_BOUNDARY 487 // Lowest resistance
uint32_t target_voltage = SETPOINT;

//  Variables for UART
int32_t buffer_voltage_mv;
uint32_t resistance;
uint32_t pwr_pw;

//  GPIO pins
struct gpio_dt_spec blue_led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(usrled0), gpios);
struct gpio_dt_spec red_led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(usrled1), gpios);
struct gpio_dt_spec relay6_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay6), gpios);

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
    gpio_pin_configure_dt(&relay6_pin, GPIO_OUTPUT_INACTIVE);

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

                switch (cmd)
                {
                case 1:
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

                // peripherals_set_digital_potentiometer(pot_val);


                uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 10000);
                

            } else {
                printk("Invalid frame\n");
            }

            new_uart_message = false;
        }



        // Timer event handling
        if(timer_elapsed == 1){ // UPDATE --> DIT GEBRUIKEN
            on_timer_event();
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
        if(pot_val < 232){
            resistance = (256 - pot_val) * 1000000 / 256;
            #ifdef CALIBRATION
            // resistance = resistance - (20.60615 * pot_val - 5506.045);
            resistance = resistance - (1.227270e+00 * pot_val * pot_val + -4.248847e+02 * pot_val + 2.791696e+04);
            #endif
        }
        else{
            resistance = (256 - (pot_val - 232)) * 94300 / 256;
            resistance = (resistance * 1e6)/(resistance + 1e6);
            #ifdef CALIBRATION
            resistance = resistance - (1.008981e-02 * pot_val * pot_val + -5.906308e-01 * pot_val + -2.188078e+03);
            #endif
        }

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