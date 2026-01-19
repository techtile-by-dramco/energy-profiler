#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include "advertisement.h"


// Global adv params buffer
struct adv_params ap;

// Static functions
static void print_buffer(uint8_t *buffer, uint8_t len);

static void copy_to_adv_buffer(){
	struct bt_data ad_variable;
	ad_variable.type = BT_DATA_MANUFACTURER_DATA;
	ad_variable.data = ap.usr_tx_buffer;
	ad_variable.data_len = sizeof(ap.usr_tx_buffer);

	memcpy(ap.adv_buffer, ad, sizeof(ad));
	ap.adv_buffer[DEFAULT_SIZE_ADV_MSG] = ad_variable;

	// Update advertisement packet
	ap.update = true;
}

static void adv_init(void){
	ap.update = true;

	copy_to_adv_buffer();

	/* Initialize the Bluetooth Subsystem */
	bt_enable(NULL);
	printk("Bluetooth initialized\n");

}

// Start advertisements
void adv_start(void){

	adv_init();

	while (1)
	{
		// // Check every 150 ms * 5
		// k_msleep(150*5);


		if(ap.update){
			ap.update = false;

			k_msleep(90);

			gpio_pin_set_dt(&blue_led_pin, 1);

			// Update measured voltages and currents
			adv_update_adc_readings();
			
			// Start advertising (*** BT_LE_ADV_NCONN --> 150ms *** BT_LE_ADV_CUSTOM --> 1000 ms adv interval)
			bt_le_adv_start(BT_LE_ADV_NCONN, ap.adv_buffer, ARRAY_SIZE(ap.adv_buffer), NULL, 0);

			k_msleep(10);

			// Stop advertising
			bt_le_adv_stop();

			gpio_pin_set_dt(&blue_led_pin, 0);
		}
	}
}

void adv_update_adc_readings(void){
	uint32_t adc_send_buf[3];
	if(buffer_voltage_mv > 0)	{ adc_send_buf[0] = (uint32_t)(buffer_voltage_mv);	} else { adc_send_buf[0] = 0; }
	if(resistance > 0)				{ adc_send_buf[1] = (uint32_t)(resistance);					} else { adc_send_buf[1] = 0; }
	if(pwr_nw > 0)	 					{ adc_send_buf[2] = (uint32_t)(pwr_nw);							} else { adc_send_buf[2] = 0; }

	for(uint8_t i = 0; i < 3; i++)
		for(uint8_t j = 0; j < 4; j++)
			ap.usr_tx_buffer[i*4 + j] = (uint8_t)(adc_send_buf[i] >> 8 * (3-j));

	// print_buffer(ap.usr_tx_buffer, ADV_TX_BUF_LEN);

	copy_to_adv_buffer();
}

static void print_buffer(uint8_t *buffer, uint8_t len){
	// Print full message
	for (size_t i = 0; i < len; i++) {
    printk("%02X ", *(buffer + i));
  }
	printk("\n");	
}