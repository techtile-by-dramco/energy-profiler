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
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/uart.h>

//  Include external I2C peripherals
#include "peripherals/peripherals.h"

// Declare the timer
struct k_timer my_timer;
uint8_t timer_elapsed = 0;

// #include "advertisement/advertisement.h"

uint16_t delay_on_ms = 500;
uint16_t delay_off_ms = 500;

// uint8_t pot_val = 255; // for safety start with lowest resistance
int32_t pot_val;
#define LOWER_BOUNDARY 0 
#define UPPER_BOUNDARY 487//255 // Lowest resistance

int32_t buffer_voltage_mv;
uint32_t resistance;
uint32_t pwr_pw;

uint8_t counter = 0;

//  GPIO pins
struct gpio_dt_spec blue_led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(usrled0), gpios);
struct gpio_dt_spec red_led_pin = GPIO_DT_SPEC_GET(DT_ALIAS(usrled1), gpios);
struct gpio_dt_spec relay1_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay1), gpios);
struct gpio_dt_spec relay2_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay2), gpios);
struct gpio_dt_spec relay3_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay3), gpios);
struct gpio_dt_spec relay4_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay4), gpios);
struct gpio_dt_spec relay5_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay5), gpios);
struct gpio_dt_spec relay6_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay6), gpios);
struct gpio_dt_spec relay7_pin = GPIO_DT_SPEC_GET(DT_ALIAS(relay7), gpios);

// static void print_byte_buffer(const unsigned char *buffer, size_t length);
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

static uint8_t calculate_xor_checksum(const uint8_t* buffer, size_t length);

//  Create UART device
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

const struct uart_config uart_cfg = {
	.baudrate = 115200,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define ADV_PRIORITY 7
#define DBG_PRIORITY 8

static int debugger();

// *** START THREADS *** //
K_THREAD_DEFINE(debugger_id, STACKSIZE, debugger, NULL, NULL, NULL, DBG_PRIORITY, 0, 0);

#define KP 0.02     // Proportional gain
// #define KI 0.1      // Integral gain
// #define KD 0        // Derivative gain

#define SETPOINT    1800    //  mV
#define SP_ACCURAY  50      //  mV

uint32_t upper_set_point = SETPOINT + SP_ACCURAY;
uint32_t lower_set_point = SETPOINT - SP_ACCURAY;

// Other global variables
uint32_t num = 0;
uint32_t max_voltage = 0;

#define LOOP


// Timer callback function
void my_timer_handler(struct k_timer *dummy) {
    timer_elapsed = 1;
    // on_timer_event();
}

// Define uint8_t array to store converted values
uint8_t buf8 [15];
uint32_t buf32 [3]; 

void on_timer_event(){
    // Reset timer elapsed variable
    timer_elapsed = 0;

    // Update uint32_t array
    buf32 [0] = buffer_voltage_mv;
    buf32 [1] = resistance;
    buf32 [2] = pwr_pw;

    // Calculate total number of elements in buf32 array
    uint8_t buf32_size = sizeof(buf32) / sizeof(buf32[0]);

    // Begin delimter + length
    buf8[0] = 0x02;
    buf8[1] = buf32_size * sizeof(uint32_t) + 1;

    // Perform conversion
    size_t idx = 2;
    for (uint8_t i = 0; i < 3; i++) {
        // Extract each 32-bit value into 4 bytes
        buf8[idx++] = (uint8_t)(buf32[i] >> 24);  // Most significant byte (MSB)
        buf8[idx++] = (uint8_t)(buf32[i] >> 16);
        buf8[idx++] = (uint8_t)(buf32[i] >> 8);
        buf8[idx++] = (uint8_t)(buf32[i]);        // Least significant byte (LSB)
    }

    buf8[14] = calculate_xor_checksum(buf8, 14);

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
    gpio_pin_configure_dt(&relay1_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay1_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay2_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay3_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay4_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay5_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay6_pin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay7_pin, GPIO_OUTPUT_INACTIVE);

    // Init peripherals (ADS and programmable load)
    perihperals_init();

    peripherals_set_digital_potentiometer(pot_val);

    int err;

    /*          Enable UART         */
	if (!device_is_ready(uart)) {
		return 0;
	}
	
	printk("UART is ready");

	err = uart_configure(uart, &uart_cfg);

	if (err == -ENOSYS) {
		return -ENOSYS;
	}

	err = uart_callback_set(uart, uart_cb, NULL);
	if (err) {
		return err;
	}

    // Initialize the timer with the handler function and set interval
    k_timer_init(&my_timer, my_timer_handler, NULL);

    #define SAMPLE_FREQ     250
    #define TIMER_PERIOD    1000/SAMPLE_FREQ //2

    /* start a periodic timer that expires once every second */
    k_timer_start(&my_timer, K_MSEC(TIMER_PERIOD), K_MSEC(TIMER_PERIOD));

    #ifdef LOOP


    while(1){
        
        if(timer_elapsed == 1){ // UPDATE --> DIT GEBRUIKEN
            on_timer_event();
        }

        // gpio_pin_set_dt(&blue_led_pin, 1);
        // gpio_pin_set_dt(&red_led_pin, 0);
        // k_msleep(delay_off_ms);
        // gpio_pin_set_dt(&blue_led_pin, 0);
        // gpio_pin_set_dt(&red_led_pin, 1);
        // k_msleep(delay_on_ms);
        // printk("test");

        k_usleep(500); // UPDATE --> DIT WEG

        // peripherals_ads1115_read_one_channel(0);
        // buffer_voltage_mv = peripherals_ads1115_read(ADS1115_COMP_0_GND);

        int32_t voltage = peripherals_ads1115_read(ADS1115_COMP_0_GND);

        if (voltage > 0) {
            buffer_voltage_mv = voltage;
        }

        // k_usleep(5000);

        // buffer_voltage_mv = peripherals_ads1115_read(ADS1115_COMP_0_GND);

        //  Calculate resistance value
        if(pot_val < 232){
            resistance = (256 - pot_val) * 1000000 / 256;
        }
        else{
            resistance = (256 - (pot_val - 232)) * 94300 / 256;
            resistance = (resistance * 1e6)/(resistance + 1e6);
        }

        //  Calculate pwr level
        pwr_pw = buffer_voltage_mv*buffer_voltage_mv*1e6 / resistance;


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
            uint8_t error = (buffer_voltage_mv - upper_set_point)*KP;
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
            uint8_t error = (lower_set_point - buffer_voltage_mv)*KP;
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
    }

    #endif

    #ifndef LOOP

    while(1){
 
        if(pot_val > 450) pot_val = 0;

        
        peripherals_set_digital_potentiometer(pot_val);


        if(pot_val < 232){
            resistance = (256 - pot_val) * 1000000 / 256;
        }
        else{
            resistance = (256 - (pot_val - 232)) * 94300 / 256;
            resistance = (resistance * 1e6)/(resistance + 1e6);
        }


        k_msleep(5000);

        pot_val = pot_val + 50;

    }

    #endif

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

    int err;

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
}


static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {
	
	case UART_TX_DONE:
		// do something
		break;

	case UART_TX_ABORTED:
		// do something
		break;
		
	case UART_RX_RDY:
		// do something
		break;

	case UART_RX_BUF_REQUEST:
		// do something
		break;

	case UART_RX_BUF_RELEASED:
		// do something
		break;
		
	case UART_RX_DISABLED:
		// do something
		break;

	case UART_RX_STOPPED:
		// do something
		break;
		
	default:
		break;
	}
}