/*

File    : uart_e-puck2.h
Author  : Stefano Morgani
Date    : 10 January 2018
REV 1.0

Functions to configure and use the UART communication between the ESP32 and both the main processor (F407) and the programmer (F413).
*/

#ifndef UART_E_PUCK_2_H
#define UART_E_PUCK_2_H

#define UART_TX_BUFF_SIZE 22
#define UART_RX_BUFF_SIZE 104
#define RESPONSE_SIZE 103

#define ECHO_TASK_STACK_SIZE	1024
#define ECHO_TASK_PRIO			10

typedef enum {
	SENSORS_BUFF_EMPTY,
	SENSORS_BUFF_FILLED
} sensors_buffer_state_t;

typedef struct {
	sensors_buffer_state_t state;
	uint8_t* data;
} sensors_buffer_t;

/**
 * @brief Task used to exchange data with the F407 through UART (based on asercom protocol).
 *
 * @param *pvParameter	parameter from the xCreateTask 	
 */
void advsercom_task(void *pvParameter);

/**
 * @brief 	UART initialization.
 *
 */
void uart_init(void);

/**
 * @brief Set the buffer content that will be sent to the F407 the next time.
 */
void uart_set_actuators_state(uint8_t *buff);

/**
 * @brief Return the pointer to the most recent UART data received from the F407.
 */
sensors_buffer_t *uart_get_data_ptr(void);

void uart_set_left_speed(int16_t left_speed);
void uart_set_right_speed(int16_t right_speed);
void uart_set_motors_speed(int16_t left_speed, int16_t right_speed);
void uart_set_leds(uint8_t leds);
void uart_set_rgb_led2(uint8_t red, uint8_t green, uint8_t blue);
void uart_set_rgb_led4(uint8_t red, uint8_t green, uint8_t blue);
void uart_set_rgb_led6(uint8_t red, uint8_t green, uint8_t blue);
void uart_set_rgb_led8(uint8_t red, uint8_t green, uint8_t blue);
void uart_set_rgb_leds(uint8_t *rgb_values);
void uart_set_sound(uint8_t sound);
void uart_set_sound_freq(uint16_t frequency);
void uart_get_proximity(uint8_t *prox_data);
void uart_get_mic(uint8_t *data);
void uart_get_distance(uint8_t *data);
void uart_get_sd_state(uint8_t *data);
void uart_get_acc_raw(uint8_t *data);
void uart_get_battery(uint8_t *data);
void uart_get_gyro_raw(uint8_t *data);
void uart_get_tv_remote(uint8_t *data);
void uart_get_selector(uint8_t *data);
void uart_set_all_actuators(uint8_t *data);
void uart_get_all_sensors(uint8_t *data);
int8_t uart_get_mic_data(uint8_t *data);
bool uart_is_transparent_mode(void);

// Camera image modes in the header of the asercom -'I' response.
#define UART_IMG_MODE_GREY	0
#define UART_IMG_MODE_RGB565	1

/**
 * @brief Request a camera image to the F407 with the asercom binary command -'I' (selector 3) and wait for it.
 *        The image format is set on the F407 side (default 40x40 RGB565, at most 4136 bytes).
 * @param dst		destination buffer, must not be in use until the function returns.
 * @param mode		filled with the image mode (UART_IMG_MODE_GREY or UART_IMG_MODE_RGB565), can be NULL.
 * @param width		filled with the image width, can be NULL.
 * @param height	filled with the image height, can be NULL.
 * @return number of bytes received, -1 on error (timeout, image bigger than dst_len, Bluetooth connected).
 */
int32_t uart_get_image(uint8_t *dst, uint32_t dst_len, uint8_t *mode, uint8_t *width, uint8_t *height);

/**
 * @brief Set the camera image format on the F407 with the asercom command "J" (selector 3).
 *        The parameters must be already validated: the F407 doesn't report configuration errors.
 * @param mode		UART_IMG_MODE_GREY or UART_IMG_MODE_RGB565.
 * @param zoom		subsampling factor (1, 2, 4): the image covers (width*zoom)x(height*zoom) sensor pixels.
 * @param x1, y1	top-left corner of the area on the sensor (640x480), -1 to center it.
 * @return 0 if the F407 confirmed the command, -1 otherwise.
 */
int8_t uart_set_camera(uint8_t mode, uint8_t width, uint8_t height, uint8_t zoom, int16_t x1, int16_t y1);

#endif /* UART_E_PUCK_2_H */