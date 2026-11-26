/*
 * fsm_uart.c
 *
 *  Created on: Nov 18, 2026
 *      Author: trungvu
 */
#include "fsm_uart.h"
#include "fsm_parser.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

typedef enum {
    UART_IDLE,
    UART_SEND_ADC,
    UART_WAIT_OK
} UartState_t;

static UartState_t uart_state = UART_IDLE;
static uint32_t adc_value = 0;
static uint32_t timer_start = 0;
static char tx_buffer[30];

void uart_fsm(void)
{
    switch (uart_state)
    {
        case UART_IDLE:
            if (command_flag == 1)
            {
                command_flag = 0;
                if (command_data == CMD_RST){
                    adc_value = HAL_ADC_GetValue(&hadc1);//current ADC value once when !RST# is received
                    uart_state = UART_SEND_ADC;
                }
            }
            break;

        case UART_SEND_ADC:
        	parser_clear_partial_line();
            int len = sprintf(tx_buffer, "!ADC=%lu#\r\n", adc_value);
            HAL_UART_Transmit(&huart2, (uint8_t *)tx_buffer, len, 100);
            parser_restore_partial_line();

            timer_start = HAL_GetTick();//start 3 sec count down !!!!!!!!!!
            uart_state = UART_WAIT_OK;
            break;


        case UART_WAIT_OK:
            if (command_flag == 1){
                command_flag = 0;
                if (command_data == CMD_OK){
                    uart_state = UART_IDLE; //stop resending and return to idl
                }else if (command_data == CMD_RST){
                    /* New request received before !OK#: sample new ADC and send */
                    adc_value = HAL_ADC_GetValue(&hadc1);
                    uart_state = UART_SEND_ADC;
                }
            }
            else if ((HAL_GetTick() - timer_start) >= 3000){
                //resend
                uart_state = UART_SEND_ADC;
            }
            break;

        default:
            uart_state = UART_IDLE;
            break;
    }
}

