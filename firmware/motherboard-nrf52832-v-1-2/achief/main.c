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

//  Include external I2C peripherals
#include "peripherals/peripherals.h"

#include "advertisement/advertisement.h"

#include "nrf.h"

uint16_t delay_on_ms = 500;
uint16_t delay_off_ms = 500;

// uint8_t pot_val = 255; // for safety start with lowest resistance
uint16_t pot_val;
#define LOWER_BOUNDARY 0 
#define UPPER_BOUNDARY 255//487//255 // Lowest resistance

int32_t buffer_voltage_mv;
uint32_t resistance;
uint32_t pwr_nw;

// uint8_t counter = 0;

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

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define ADV_PRIORITY 7
#define DBG_PRIORITY 8

static void debugger();

// *** START THREADS *** //
K_THREAD_DEFINE(start_adv_id, STACKSIZE, adv_start, NULL, NULL, NULL, ADV_PRIORITY, 0, 0);
K_THREAD_DEFINE(debugger_id, STACKSIZE, debugger, NULL, NULL, NULL, DBG_PRIORITY, 0, 0);

#define KP 0.01     // Proportional gain
// #define KI 0.1      // Integral gain
// #define KD 0        // Derivative gain

#define SETPOINT    1600    //  mV
#define SP_ACCURAY  50      //  mV

uint32_t upper_set_point = SETPOINT + SP_ACCURAY;
uint32_t lower_set_point = SETPOINT - SP_ACCURAY;

// Other global variables
uint32_t num = 0;
uint32_t max_voltage = 0;

#define LOOP

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


    // while(1){
        
    //     if(pot_val > 300) pot_val = 0;

    //     pot_val = pot_val + 100;

    //     // pot_val = 0;
        
    //     peripherals_set_digital_potentiometer(pot_val);

    //     k_msleep(5000);
    // }
    

    uint32_t reset_reason = NRF_POWER->RESETREAS;



    printk("RESET: %d \n", reset_reason);

    NRF_POWER->RESETREAS = 0xFFFFFFFF;

    k_msleep(5000);




    #ifdef LOOP

    while(1){
        // gpio_pin_set_dt(&blue_led_pin, 1);
        // gpio_pin_set_dt(&red_led_pin, 0);
        // k_msleep(delay_off_ms);
        // gpio_pin_set_dt(&blue_led_pin, 0);
        // gpio_pin_set_dt(&red_led_pin, 1);
        // k_msleep(delay_on_ms);


        // printk("test");

        // peripherals_ads1115_read_one_channel(0);
        buffer_voltage_mv = peripherals_ads1115_read(ADS1115_COMP_0_GND);
        k_msleep(1);
        //k_usleep(500);

        if(buffer_voltage_mv > 2000){
            num++;
        }

        if(max_voltage < buffer_voltage_mv){
            max_voltage = buffer_voltage_mv;
        }

        resistance = (256 - pot_val) * 1000000 / 256;

        pwr_nw = buffer_voltage_mv*buffer_voltage_mv*1000 / resistance;

        //  Reset performance measurements
        if(pot_val == 0) { num = 0; max_voltage = 0; }

        if(buffer_voltage_mv > upper_set_point){
            uint16_t error = (uint8_t)((buffer_voltage_mv - upper_set_point)*KP);
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
        if(buffer_voltage_mv < lower_set_point){
            uint16_t error = (uint8_t)((lower_set_point - buffer_voltage_mv)*KP);
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

    return 0;
}


void debugger(){

    while(1){

        k_msleep(500);

        printk("Buffer voltage: %d mV\n", buffer_voltage_mv);

        printk("Pot value: %d\n", pot_val);

        printk("resistance: %d\n", resistance);

        printk("Power: %d nW\n", pwr_nw);

        // printk("%d\n", num);

        printk("Approximated time above 2V: %d ms\n", num*947/1000); // loop speed (947 us) measured with logic analyser

        printk("Max. voltage measured: %d mV\n", max_voltage);

    }
}