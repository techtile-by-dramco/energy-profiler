

#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/drivers/uart.h>

// #include <stderr.h>
#include <stdint.h>
#include <stddef.h>

#include "stdio.h"

// enable UART transmission
#define ENABLE_UART_TX
// #define PRINT_ENABLE

int32_t buffer_voltage_mv;
uint32_t resistance;
uint32_t pwr_nw;

//  Data buffer to send over UART
#define NO_VARS 	3
uint8_t raw_data_buffer [NO_VARS*4 + 2];

int observer_start(void);

//  Import LED
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// static void print_byte_buffer(const unsigned char *buffer, size_t length);
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

//  Create UART device
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

const struct uart_config uart_cfg = {
	.baudrate = 115200,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};


int main(void){

    int err;

    printk("Start");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

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
	/*      *** *** *** *** ***     */

    // Begin delimter + length
    raw_data_buffer[0] = 0x02;
    raw_data_buffer[1] = 2 + NO_VARS*4;

    // Start observer
    (void)observer_start();

    while(1){
        gpio_pin_toggle_dt(&led);
		k_msleep(250);

        #ifdef PRINT_ENABLE
            printk("buffer_voltage_mv: %"PRId32" mV\n", buffer_voltage_mv);
            printk("resistance: %"PRId32" ohm\n", resistance);
            printk("pwr_nw: %"PRId32" nW\n", pwr_nw);
        #endif

        #ifdef ENABLE_UART_TX
            err = uart_tx(uart, raw_data_buffer, sizeof(raw_data_buffer), SYS_FOREVER_US);
            if (err) {
                return err;
            }
        #endif

    }

    return 0;
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