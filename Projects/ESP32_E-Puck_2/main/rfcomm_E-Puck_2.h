/*

File    : rfcomm_E-Puck_2.c
Author  : Eliot Ferragni
Date    : 17 november 2017
REV 1.2

Functions to control and use the bluetooth stack
*/


#ifndef RFCOMM_E_PUCK2_H
#define RFCOMM_E_PUCK2_H

#include "btstack.h"


//#define ENABLE_LOG_RFCOMM

#define BLUE_RX_BUFFER_SIZE		2000	
#define BLUE_TX_BUFFER_SIZE		2000

/**
 *  Hardware state of Bluetooth controller 
 */
typedef enum {
	DISABLE = 0,
	ENABLE,
} CONTROL_STATE;

typedef enum{
	NO_UPDATE = 0,
	UPDATE,
} UPDATE_STATE;

// #ref returns bluetooth functions
#define DATAS_WRITTEN			0
#define BUFFER_FULL				-1
#define TASK_COLLIISION			-2
#define BLUETOOTH_NOT_CONNECTED	-3

#ifdef ENABLE_LOG_RFCOMM
#define log_rfcomm(format, ...)  printf(format,  ## __VA_ARGS__)
#else
#define log_rfcomm(...) __log_unused(__VA_ARGS__)
#endif

/**
 * @brief Write datas to the bluetooth
 *        To achieve best performances, it is best to provide a large amount of datas at the same time
 *        and to wait minimum 10ms between two calls of this function.
 *
 * @param buffer 		Pointer to a buffer containing the datas to send
 * @param buffer_len	Size of the buffer provided (max BLUE_TX_BUFFER_SIZE)
 *
 * @return		A status code. See #ref return bluetooth functions
 */
int8_t bluetooth_write(uint8_t* buffer, uint16_t buffer_len);

/**
 * @brief Read datas from the bluetooth
 *        To achieve best performances, it is best to provide a large buffer
 *        in order to read more at the same time and to wait minimum 10ms between two calls of this function
 *
 * @param buffer 		Pointer to a buffer to feed with the received datas
 * @param buffer_len	Size of the buffer provided
 *
 * @return		The amount of datas read or a status code. See #ref returns bluetooth functions
 */
int16_t bluetooth_read(uint8_t* buffer, uint16_t buffer_len);

/**
 * @brief Enable/disable the power of the bluetooth module
 *
 * @param power_mode 	Power mode desired. See HCI_POWER_MODE
 */
void bluetooth_power_control(HCI_POWER_MODE power_mode);

/**
 * @brief 	Enable/disable the discoverability of the bluetooth
 * 			The discoverability is only usefull for the paring. Once paired, we can connect
 * 			to the bluetooth without seeing it.
 * 			It consumes less energy when the discoverability is disabled.
 * 			The power_mode HCI_POWER_SLEEP should only be used if discoverability and connectivity
 * 			are both disabled.
 *
 * @param state discoverability state desired. See CONTROL_STATE 	
 */
void bluetooth_discoverable_control(CONTROL_STATE state);

/**
 * @brief 	Enable or disable the connectivity of the bluetooth 
 * 			If disabled, no new connection is possible, even if already paired.
 * 			Usefull to consume less energy (does not disable the power) or to block a new connection 
 * 			if a connection is already in use.
 *
 * @param state connectivity state desired. See CONTROL_STATE 	
 */
void bluetooth_connectable_control(CONTROL_STATE state);

/**
 * @brief 	Simple example of an echo task over the bluetooth
 *
 * @param *pvParameter	parameter from the xCreateTask 	
 */
void example_echo_bluetooth_task(void *pvParameter);


#endif /* RFCOMM_E_PUCK2_H */