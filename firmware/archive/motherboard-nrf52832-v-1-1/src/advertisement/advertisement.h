

#ifndef ADVERTISEMENT_H_
#define ADVERTISEMENT_H_

#include "zephyr/bluetooth/bluetooth.h"
#include "zephyr/bluetooth/hci.h"

#define ADV_TX_BUF_LEN 	3*4 //7

#define BT_LE_ADV_CUSTOM BT_LE_ADV_PARAM(0, BT_GAP_PER_ADV_SLOW_INT_MIN, \
					BT_GAP_PER_ADV_SLOW_INT_MAX, NULL)

extern int32_t buffer_voltage_mv;
extern uint32_t resistance;
extern uint32_t pwr_nw;

// Filter name of incoming ADV messages
static char name [] = "end-logger";

// *** Default advertisement message content ***
static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, name, sizeof(name) - 1),
	//BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR)
};

// Default size ADV message
#define DEFAULT_STRUCT_SIZE_ADV_MSG 	sizeof(struct bt_data)
#define DEFAULT_SIZE_ADV_MSG 					(uint8_t)(sizeof(ad)/sizeof(struct bt_data))

// Define struct
struct adv_params {
    bool update;
    uint8_t usr_tx_buffer [ADV_TX_BUF_LEN];
	struct bt_data adv_buffer[DEFAULT_SIZE_ADV_MSG + 1];
};

// // Communication
// static void print_buffer(uint8_t *buffer, uint8_t len){
// 	// Print full message
// 	for (size_t i = 0; i < len; i++) {
//         printk("%02X ", *(buffer + i));
//     }
// 	printk("\n");	
// }



void adv_start();

// void adv_change_status(uint8_t stat);

void adv_update_adc_readings(void);


#endif /* ADVERTISEMENT_H_ */