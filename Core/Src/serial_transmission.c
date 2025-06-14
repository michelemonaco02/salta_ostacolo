/*
 * serial_transmissio.c
 *
 *  Created on: Jun 4, 2025
 *      Author: angio
 */

#include "serial_transmission.h"
#include "stm32f3xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>  // Per argomenti variabili

extern UART_HandleTypeDef huart2;

void send_msg(const char* format,...){
	char buffer[128];  // Puoi aumentare se necessario

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

}

