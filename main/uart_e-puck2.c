/*

File    : uart_e-puck2.h
Author  : Stefano Morgani
Date    : 10 January 2018
REV 1.0

Functions to configure and use the UART communication between the ESP32 and both the main processor (F407) and the programmer (F413).
*/
#include <string.h>

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "main_e-puck2.h"
#include "uart_e-puck2.h"
#include "rgb_led_e-puck2.h"
#include "button_e-puck2.h"
#include "rfcomm_e-puck2.h"

#define UART_407 UART_NUM_1

#define UART_STATE_SYNC 0
#define UART_STATE_SEND_STANDARD_CMD 1
#define UART_STATE_RECV_STANDARD_RESP 2
#define UART_STATE_TRANSPARENT_MODE 3
#define UART_STATE_SEND_SPECIAL_CMD 4
#define UART_STATE_RECV_SPECIAL_RESP 5

const int EVT_SENSORS_BUFF_FILL_NEXT = BIT0;
const int EVT_SENSORS_BUFF_FILLED = BIT1;

const int EVT_RESP_SPECIAL_OK = BIT0;
const int EVT_RESP_SPECIAL_FAIL = BIT1;

uint8_t uart_tx_buff[UART_TX_BUFF_SIZE]; // Tx to F407.
sensors_buffer_t* uart_rx_buff1; // Rx from F407.
sensors_buffer_t* uart_rx_buff2;
sensors_buffer_t* uart_rx_buff_last;
sensors_buffer_t* uart_rx_buff_curr;
uint8_t uart_transparent_mode = 0;
static EventGroupHandle_t uart_event_group;
uint8_t uart_tx_buff_special[UART_TX_BUFF_SIZE] = {0};
uint8_t cmd_special_size = 0;
uint8_t send_special_cmd = 0;
uint16_t resp_special_size = 0;
uint8_t* uart_rx_buff_special = NULL;
static EventGroupHandle_t cmd_special_group;
uint8_t uart_tx_end_delim[3] = {'\r', '\n', 0x00};

void uart_set_actuators_state(uint8_t *buff) {
	// uart_tx_buff[2] = buff[2]; // Behaviors/others
	// uart_tx_buff[3] = buff[3]; // Left speed / left steps LSB
	// uart_tx_buff[4] = buff[4]; // Left speed / left steps MSB
	// uart_tx_buff[5] = buff[5]; // Right speed / right steps LSB
	// uart_tx_buff[6] = buff[6]; // Right speed / right steps MSB
	// uart_tx_buff[7] = buff[7]; // LEDs
	// uart_tx_buff[8] = buff[20];	// Sound.
	
	// rgb_update_all(&buff[8]);
	
	memcpy(&uart_tx_buff[2], &buff[2], 19);
}

void uart_set_left_speed(int16_t left_speed) {
	uart_tx_buff[3] = (uint8_t)(left_speed & 0x00FF); // Left speed / left steps LSB
	uart_tx_buff[4] = (uint8_t)((left_speed >> 8) & 0x00FF); // Left speed / left steps MSB
	uart_get_data_ptr();
}

void uart_set_right_speed(int16_t right_speed) {
	uart_tx_buff[5] = (uint8_t)(right_speed & 0x00FF); // Right speed / right steps LSB
	uart_tx_buff[6] = (uint8_t)((right_speed >> 8) & 0x00FF); // Right speed / right steps MSB
	uart_get_data_ptr();
}

void uart_set_motors_speed(int16_t left_speed, int16_t right_speed) {
	if(left_speed > 1000) left_speed = 1000;
	if(left_speed < -1000) left_speed = -1000;
	if(right_speed > 1000) right_speed = 1000;
	if(right_speed < -1000) right_speed = -1000;
	uart_tx_buff[3] = (uint8_t)(left_speed & 0x00FF); // Left speed / left steps LSB
	uart_tx_buff[4] = (uint8_t)((left_speed >> 8) & 0x00FF); // Left speed / left steps MSB
	uart_tx_buff[5] = (uint8_t)(right_speed & 0x00FF); // Right speed / right steps LSB
	uart_tx_buff[6] = (uint8_t)((right_speed >> 8) & 0x00FF); // Right speed / right steps MSB
	uart_get_data_ptr();
}

void uart_set_leds(uint8_t leds) {
	uart_tx_buff[7] = leds;
	uart_get_data_ptr();
}

void uart_set_rgb_led2(uint8_t red, uint8_t green, uint8_t blue) {
	uart_tx_buff[8] = red;
	uart_tx_buff[9] = green;
	uart_tx_buff[10] = blue;
	uart_get_data_ptr();
}

void uart_set_rgb_led4(uint8_t red, uint8_t green, uint8_t blue) {
	uart_tx_buff[11] = red;
	uart_tx_buff[12] = green;
	uart_tx_buff[13] = blue;
	uart_get_data_ptr();
}

void uart_set_rgb_led6(uint8_t red, uint8_t green, uint8_t blue) {
	uart_tx_buff[14] = red;
	uart_tx_buff[15] = green;
	uart_tx_buff[16] = blue;
	uart_get_data_ptr();
}

void uart_set_rgb_led8(uint8_t red, uint8_t green, uint8_t blue) {
	uart_tx_buff[17] = red;
	uart_tx_buff[18] = green;
	uart_tx_buff[19] = blue;
	uart_get_data_ptr();
}

void uart_set_rgb_leds(uint8_t *rgb_values) {
	memcpy(&uart_tx_buff[8], rgb_values, 12);
	uart_get_data_ptr();
}

void uart_set_sound(uint8_t sound) {
	uart_tx_buff[20] = sound;
	uart_get_data_ptr();
}

void uart_set_all_actuators(uint8_t *data) {
	memcpy(&uart_tx_buff[2], &data[0], 19);
	uart_get_data_ptr();
}

void uart_set_sound_freq(uint16_t frequency) {
	memset(uart_tx_buff_special, 0, UART_TX_BUFF_SIZE);
	uart_tx_buff_special[0] = -0x13;
	uart_tx_buff_special[1] = frequency & 0xFF;
	uart_tx_buff_special[2] = (frequency >> 8) & 0xFF;
	cmd_special_size = 4;
	send_special_cmd = 1;
	resp_special_size = 0;
	uart_get_data_ptr();
}

sensors_buffer_t *uart_get_data_ptr(void) {
	// Wait for the next buffer to be filled in case it isn't.
	if(uart_rx_buff_curr->state == SENSORS_BUFF_EMPTY) {
		//rgb_led2_gpio_set(0, 0, 0);
		//printf("wait\r\n");
		xEventGroupWaitBits(uart_event_group, EVT_SENSORS_BUFF_FILLED, true, false, portMAX_DELAY);
	}
	//printf("get filled\r\n");
	xEventGroupClearBits(uart_event_group, EVT_SENSORS_BUFF_FILLED); // This bit remain set if the buffer was already filled, so clear it.
	
	uart_rx_buff_last = uart_rx_buff_curr;
	if(uart_rx_buff_curr == uart_rx_buff1) {
		uart_rx_buff_curr = uart_rx_buff2;
	} else {
		uart_rx_buff_curr = uart_rx_buff1;
	}
	uart_rx_buff_curr->state = SENSORS_BUFF_EMPTY;
	
	xEventGroupSetBits(uart_event_group, EVT_SENSORS_BUFF_FILL_NEXT); // Tell the SPI task to fill the next buffer.
	//printf("set fill next\r\n");
	
	return uart_rx_buff_last;
}

void uart_get_proximity(uint8_t *prox_data) {
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(prox_data, &buff->data[37], 16);
}

void uart_get_mic(uint8_t *data) 
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[71], 8);
}

void uart_get_distance(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[69], 2);
}

void uart_get_sd_state(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[85], 1);
}

void uart_get_acc_raw(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[0], 6);
}

void uart_get_battery(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[83], 2);
}

void uart_get_gyro_raw(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[18], 6);
}

void uart_get_tv_remote(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[88], 1);
}

void uart_get_selector(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[89], 1);
}

void uart_get_all_sensors(uint8_t *data)
{
	sensors_buffer_t* buff = uart_get_data_ptr();
	memcpy(data, &buff->data[0], 103);
}

int8_t uart_get_mic_data(uint8_t *data)
{
	EventBits_t uxBits;
	memset(uart_tx_buff_special, 0, UART_TX_BUFF_SIZE);
	uart_tx_buff_special[0] = -'U';
	cmd_special_size = 2;
	send_special_cmd = 1;
	resp_special_size = 1280;
	uart_get_data_ptr();	
	uart_rx_buff_special = data;
	uxBits = xEventGroupWaitBits(cmd_special_group, EVT_RESP_SPECIAL_OK|EVT_RESP_SPECIAL_FAIL, true, false, portMAX_DELAY);
	if ((uxBits & EVT_RESP_SPECIAL_OK) != 0) 
	{
        return 0;
    } 
	else 
	{
        return -1;
    }	
}

bool uart_is_transparent_mode(void)
{
	return (uart_transparent_mode == 1);
}

void advsercom_task(void *pvParameter) {
	uint8_t uart_state = UART_STATE_SEND_STANDARD_CMD;
	
	uart_rx_buff_curr = uart_rx_buff1;
	
	int len = 0;
	int total_len = 0;
	int flush_len = 0;
	int flush_tot_len = 0;
	uint8_t flush_byte = 0;
	uint8_t temp = 0;
	uint8_t rx_fail = 0;
	
	// Prepare the requests based on the asercom protocol.
	memset(uart_tx_buff, 0, UART_TX_BUFF_SIZE);
	uart_tx_buff[0]=-0x08;	// (0xF8) All sensors request.
	uart_tx_buff[1]=-0x09;	// (0xF7) All actuators set command.		
	uart_tx_buff[UART_TX_BUFF_SIZE-1]=0;	// Terminator.
	
	while(1) {		
		switch(uart_state) {
			case UART_STATE_SYNC: // Sync state (not used for now).
				//printf("sync...\r\n");
				uart_tx_chars(UART_407, (char*)&uart_tx_end_delim[1], 1);
				flush_tot_len = 0;
				while(1) {
					flush_len = uart_read_bytes(UART_407, &flush_byte, 1, 5/portTICK_PERIOD_MS);
					if(flush_len == 0) {
						break;
					}
					flush_tot_len += flush_len;
				}
				//printf("sync flush=(%d)\r\n", flush_tot_len);
				uart_state = UART_STATE_SEND_STANDARD_CMD;
				break;

			case UART_STATE_SEND_STANDARD_CMD: // Send requests and commands.
				//rgb_led2_gpio_set(0, 0, 1);	
				// Send the requests based on asercom protocol.
				// Beware: from various tests resulted that at most 24 bytes are correctly sent to the F407, 
				// if you try sending more data than this, then the data are corrupted. To be analysed deeper...
				//if(uart_tx_buff[UART_TX_BUFF_SIZE-2]!=0) {
				//	printf("wrong cmd %d!\r\n", uart_tx_buff[UART_TX_BUFF_SIZE-2]);
				//}
				len = uart_tx_chars(UART_407, (char*)&uart_tx_buff[0], UART_TX_BUFF_SIZE);
				//printf("sending...\r\n");
				//uart_wait_tx_done(UART_407, portMAX_DELAY);	
				if(uart_wait_tx_done(UART_407, 100/portTICK_PERIOD_MS) != ESP_OK) {
					printf("error on tx done!\r\n");
					break;
				}
				uart_state = UART_STATE_RECV_STANDARD_RESP;						
				//printf("sent=(%d)\r\n", len);
				break;
				
			case UART_STATE_RECV_STANDARD_RESP: // Receive sensors data.	
				//rgb_led2_gpio_set(0, 1, 0);
				// Read the expected bytes from the requests previously made.
				len = uart_read_bytes(UART_407, uart_rx_buff_curr->data, RESPONSE_SIZE, 50/portTICK_PERIOD_MS);
					
				// Flush the input buffer in case more data than expected are received.
				flush_len = 0;
				flush_tot_len = 0;
				while(1) {
					flush_len = uart_read_bytes(UART_407, &flush_byte, 1, 5/portTICK_PERIOD_MS);
					if(flush_len == 0) {
						break;
					}
					flush_tot_len += flush_len;
				}
				
				//printf("flush=(%d)\r\n", flush_tot_len);
				//printf("len=(%d)\r\n", len);
				
				// If all is ok, mark the buffer as ready.
				if(len==RESPONSE_SIZE && flush_tot_len==0) {
					rx_fail = 0;
					temp = uart_rx_buff_curr->data[UART_RX_BUFF_SIZE-2];				
					uart_rx_buff_curr->data[UART_RX_BUFF_SIZE-2] = button_is_pressed(); // Set the button state directly from here because it's logically more correct since the button is connected to the ESP32 (instead of requesting the state from the F407).
																						// Moreover the state of the button is only updated through SPI if there is a WiFi connection opened and the camera image is streamed,
																						// so its state would be updated only if also the camera is requested, that is not the correct behaviour.
					
					uart_rx_buff_curr->data[UART_RX_BUFF_SIZE-1] = temp; // Put the empty byte (reserved for future usage) at the end of the packet.
				
					uart_rx_buff_curr->state = SENSORS_BUFF_FILLED;
					xEventGroupSetBits(uart_event_group, EVT_SENSORS_BUFF_FILLED);
					//printf("set filled\r\n");					
					xEventGroupWaitBits(uart_event_group, EVT_SENSORS_BUFF_FILL_NEXT, true, false, portMAX_DELAY);
					//printf("get fill next\r\n");
				} else {
					rx_fail = 1;
					vTaskDelay(1000 / portTICK_PERIOD_MS); // Add a pause otherwise the data received by the F407 would be corrupted when the ESP32 and F407 aren't sync.
				}
				
				if(bluetoohth_is_connected())
				{
					ESP_LOGI("UART","Enter transparent mode\r\n");
					uart_state = UART_STATE_TRANSPARENT_MODE;
					uart_transparent_mode = 1;
				}
				else
				{
					if(rx_fail == 1)
					{
						uart_state = UART_STATE_SYNC;
					}
					else
						{
						if(send_special_cmd == 1)
						{
							uart_state = UART_STATE_SEND_SPECIAL_CMD;
						}
						else
						{
							uart_state = UART_STATE_SEND_STANDARD_CMD;
						}
					}
				}
				break;

			case UART_STATE_TRANSPARENT_MODE: // transparent mode when Bluetooth is connected
				vTaskDelay(100 / portTICK_PERIOD_MS);
				if(!bluetoohth_is_connected())
				{
					ESP_LOGI("UART","Exit transparent mode\r\n");
					uart_state = UART_STATE_SEND_STANDARD_CMD;
					uart_transparent_mode = 0;
				}
				break;

			case UART_STATE_SEND_SPECIAL_CMD:
				//printf("send special cmd =%d\r\n", uart_tx_buff_special[0]);
				len = uart_tx_chars(UART_407, (char*)&uart_tx_buff_special[0], cmd_special_size);
				if(uart_wait_tx_done(UART_407, 100/portTICK_PERIOD_MS) != ESP_OK) {
					//printf("error on tx done!\r\n");
					break;
				}
				if(resp_special_size > 0)
				{
					uart_state = UART_STATE_RECV_SPECIAL_RESP;	
				}
				else
				{
					uart_state = UART_STATE_SEND_STANDARD_CMD;
				}
				send_special_cmd = 0;			
				break;

			case UART_STATE_RECV_SPECIAL_RESP:
				//printf("recv special resp size=%d\r\n", resp_special_size);
				total_len = 0;
				while(total_len < resp_special_size)
				{
					//ESP_LOGI("UART","resp tot_len=%d\r\n", total_len);
					//printf("resp tot_len=%d\r\n", total_len);
					len = uart_read_bytes(UART_407, (char*)&uart_rx_buff_special[total_len], (resp_special_size-total_len), 50/portTICK_PERIOD_MS);
					if(len > 0)
					{
						total_len += len;
					}
					else
					{
						break;
					}	
				}
				
				// If all is ok, mark the buffer as ready.
				if(total_len == resp_special_size) {
					xEventGroupSetBits(cmd_special_group, EVT_RESP_SPECIAL_OK);
				} else {
					xEventGroupSetBits(cmd_special_group, EVT_RESP_SPECIAL_FAIL);
				}
				
				if(bluetoohth_is_connected())
				{
					ESP_LOGI("UART","Enter transparent mode\r\n");
					uart_state = UART_STATE_TRANSPARENT_MODE;
					uart_transparent_mode = 1;
				}
				else
				{
					if(send_special_cmd == 1)
					{
						uart_state = UART_STATE_SEND_SPECIAL_CMD;
					}
					else
					{
						uart_state = UART_STATE_SEND_STANDARD_CMD;
					}
				}	
				resp_special_size = 0;
				break;
		}
	}
}

void uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200, //2500000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    //Configure parameters.
	uart_param_config(UART_407, &uart_config);
    //UART0 uses default pins.
    //Set UART2 pins(TX: GPIO17, RX: GPIO34)
    uart_set_pin(UART_407, 17, 34, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //Install UART driver with both RX and TX buffers (we don't need an event queue here)
    uart_driver_install(UART_407, 2048, 2048, 0, NULL, 0);

	uart_rx_buff1 = (sensors_buffer_t*) malloc(sizeof(sensors_buffer_t));	
	uart_rx_buff1->data = (uint8_t*) malloc(UART_RX_BUFF_SIZE);
	if(uart_rx_buff1->data == NULL) {
		printf("cannot allocate uart rx buff1\r\n");
		return;
	} else {
		printf("uart rx buff1 allocated\r\n");
	}	
		
	uart_rx_buff2 = (sensors_buffer_t*) malloc(sizeof(sensors_buffer_t));	
	uart_rx_buff2->data = (uint8_t*) malloc(UART_RX_BUFF_SIZE);
	if(uart_rx_buff2->data == NULL) {
		printf("cannot allocate uart rx buff1\r\n");
		return;
	} else {
		printf("uart rx buff1 allocated\r\n");
	}
	
	uart_rx_buff_last = uart_rx_buff1;
	uart_rx_buff_curr = uart_rx_buff2;
	uart_rx_buff_curr->state = SENSORS_BUFF_EMPTY;	
	
	uart_event_group = xEventGroupCreate();
	cmd_special_group = xEventGroupCreate();
	
	xTaskCreatePinnedToCore(&advsercom_task, "advsercom_task", 2048, NULL, 4, NULL, CORE_0);	
}
