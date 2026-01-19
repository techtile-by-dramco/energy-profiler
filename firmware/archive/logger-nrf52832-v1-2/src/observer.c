/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#include "ringbuffer/ringbuffer.h"

// #include <bluetooth/scan.h>

// #define DEBUG_ADV_RAW_DATA

#define NAME_LEN 30

#define DEVICE_NAME "end-logger"

// Profiler device variables
extern int32_t buffer_voltage_mv;
extern uint32_t resistance;
extern uint32_t pwr_nw;

extern RingBuffer rb;

//	
#define NO_VARS 	3
extern uint8_t raw_data_buffer [14];

//	Device name buffers
char profiler_name [] = DEVICE_NAME;
char *device_name_pointer_buffer [] = {profiler_name};
uint8_t device_name_len_buffer [] = {sizeof(profiler_name)};
typedef enum{DEVICE, NONE} Devices;


// Static functions
static void extract_data(uint8_t *buffer, uint8_t len);
static void print_buffer(uint8_t *buffer, uint8_t len);


uint8_t check_device(struct net_buf_simple *ad){

	for(uint8_t device = DEVICE; device < NONE; device++){
		// Desired - buffer name rx adv
		char ds_name_buffer [] = DEVICE_NAME;
		uint16_t buffer_cmp_len = device_name_len_buffer[device] - 1;//sizeof(device_name_pointer_buffer[device]) - 1;

		// Scanned - buffer name rx adv
		char sc_name_buffer [buffer_cmp_len];
		memcpy(sc_name_buffer, ad->data + 2, buffer_cmp_len);
		
		// Print buffers to check similarity
		// print_buffer(ds_name_buffer, buffer_cmp_len);
		// print_buffer(sc_name_buffer, buffer_cmp_len);

		// Compare both buffer
		int result = memcmp(device_name_pointer_buffer[device], sc_name_buffer, buffer_cmp_len);	

		if(result == 0){
			return device;
		}
	}
	return NONE;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad){

	//	Check device name of incoming adv messages
	uint8_t dev = check_device(ad);

	#ifdef DEBUG_ADV_RAW_DATA 
		char addr_str[BT_ADDR_LE_STR_LEN];

		if(dev != NONE){
			printk("Device found: %s (RSSI %d), type %u, AD data len %u\n",
			addr_str, rssi, type, ad->len);
			print_buffer(ad->data, ad->len);
		}
	#endif

	if(dev == DEVICE){
		// Extract and save data
		extract_data(ad->data, ad->len);
	}
}


/********************************************************************/
/*												START OBSERVER														*/
/********************************************************************/

int observer_start(void){
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
	int err;


	// while(1){
	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Start scanning failed (err %d)\n", err);
		return err;
	}
	printk("Started scanning...\n");

	return 0;
}

/********************************************************************/
/*													OTHER FUNCTIONS													*/
/********************************************************************/


static void extract_data(uint8_t *buffer, uint8_t len){
	uint8_t pos = 2 + device_name_len_buffer[0] + 1; //2?

	uint32_t vars [NO_VARS];

	memcpy((raw_data_buffer + 2), (buffer + pos), NO_VARS*4);

	for(uint8_t i = 0; i < NO_VARS; i++){
		vars[i] = *(buffer + pos + i*4 + 0) << 24 | *(buffer + pos + i*4 + 1) << 16 | *(buffer + pos + i*4 + 2) << 8 | *(buffer + pos + i*4 + 3);
	}

	//	Copy to variables
	buffer_voltage_mv = vars[0];
	resistance = vars[1];
	pwr_nw = vars[2];

	add_measurement(&rb, buffer_voltage_mv, resistance, pwr_nw);

}


static void print_buffer(uint8_t *buffer, uint8_t len){
	// Print full message
	for (size_t i = 0; i < len; i++) {
    printk("%02X ", *(buffer + i));
  }
	printk("\n");	
}