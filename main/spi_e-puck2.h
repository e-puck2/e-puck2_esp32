/*

File    : spi_e-puck2.h
Author  : Stefano Morgani
Date    : 10 January 2018
REV 1.0

Functions to configure and use the SPI communication between the main processor (F407) and the ESP32. 
*/

#ifndef SPI_E_PUCK_2_H
#define SPI_E_PUCK_2_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"

#define BUF_SIZE (1024)

// Image sent by the F407 (selector 15): 160x120 RGB565, big endian.
#define SPI_IMG_WIDTH	160
#define SPI_IMG_HEIGHT	120
#define SPI_IMG_SIZE	(SPI_IMG_WIDTH*SPI_IMG_HEIGHT*2)

#define SPI_TASK_STACK_SIZE	4096
#define SPI_TASK_PRIO		5

typedef enum {
	IMG_BUFF_EMPTY=0,
	IMG_BUFF_FILLED=1
} image_buffer_state_t;

typedef struct {
	image_buffer_state_t state;
	uint8_t* data;
} image_buffer_t;

/**
 * @brief 	SPI communication handling between the F407 and ESP32
 *
 * @param *pvParameter	parameter from the xCreateTask 	
 */
void spi_task(void *pvParameter);

/**
 * @brief 	SPI initialization (VSPI port).
 *
 */
void spi_init(void);

/**
 * @brief Wait until an image with id different from "last_id" is available.
 * @return true if a new image is available, false on timeout.
 */
bool spi_wait_image(uint32_t last_id, TickType_t ticks_to_wait);

/**
 * @brief Copy the most recent complete image (SPI_IMG_SIZE bytes) into "dst".
 * @param id	filled with the id of the copied image (incremented at each image received), can be NULL.
 * @return number of bytes copied, 0 if no image is available yet, -1 if dst is too small.
 */
int32_t spi_copy_image(uint8_t *dst, uint32_t dst_len, uint32_t *id);

/**
 * @brief Number of complete images received and of images discarded (unexpected packet size).
 */
void spi_get_image_stats(uint32_t *frames, uint32_t *errors);
void spi_reset_image_stats(void);

#endif /* SPI_E_PUCK_2_H */
