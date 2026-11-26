/*
 * fsm_parser.c
 *
 *  Created on: Nov 18, 2026
 *      Author: trungvu
 */

#include "fsm_parser.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

typedef enum {
    PARSER_WAIT_START,//wait until type !
    PARSER_READ_CMD//receive char until #
} ParserState_t;

uint8_t command_flag = 0;//1 when valid cmd
uint8_t command_data = CMD_NONE;//store cmd

static ParserState_t parser_state = PARSER_WAIT_START;
static uint8_t read_index = 0;//track which byte read at buffer
static char cmd_str[10];//store the str betw ! #
static uint8_t cmd_idx = 0;//current pos inside cmd_str

static char partial_line[12];//back up when there's adc come in and interrupt the user
static uint8_t partial_len = 0;

void parser_clear_partial_line(void)
{
    if (partial_len > 0)
    {
        uint8_t cr = '\r';//\r is return the pointer to the start of the line and overwrite it
        uint8_t space = ' ';
        HAL_UART_Transmit(&huart2, &cr, 1, 50);//return the cursor to the start of the line
        for (uint8_t i = 0; i < partial_len; i++)
        {
            HAL_UART_Transmit(&huart2, &space, 1, 50);//then replace the whole typed word with space
        }
        HAL_UART_Transmit(&huart2, &cr, 1, 50);//return again
    }
}

void parser_restore_partial_line(void)//typed text reappears below the arrived !ADC#
{
    if (partial_len > 0)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)partial_line, partial_len, 50);
    }
}

void command_parser_fsm(void)
{
    while (read_index != index_buffer) {//read_index chase the index_buffer if the user type too fast or cpy paste
        uint8_t c = buffer[read_index++];
        if (read_index >= MAX_BUFFER_SIZE){
            read_index = 0;
        }

        if (c == '!'){
            if (partial_len > 0)//user type ! again, clear that line move to next line
            {
                parser_clear_partial_line();
            }
            cmd_idx = 0;
            parser_state = PARSER_READ_CMD;
            partial_line[0] = '!';
            partial_len = 1;
            HAL_UART_Transmit(&huart2, &c, 1, 50);
        } else if (parser_state == PARSER_READ_CMD){
            if (c == '#'){
                uint8_t end_seq[3] = {'#', '\r', '\n'};
                HAL_UART_Transmit(&huart2, end_seq, 3, 50);
                partial_len = 0;

                cmd_str[cmd_idx] = '\0';
                if (strcmp(cmd_str, "RST") == 0)
                {
                    command_data = CMD_RST;
                    command_flag = 1;
                }
                else if (strcmp(cmd_str, "OK") == 0)
                {
                    command_data = CMD_OK;
                    command_flag = 1;
                }

                parser_state = PARSER_WAIT_START;
            }
            else if (c != '\r' && c != '\n'){
                if (cmd_idx < (sizeof(cmd_str) - 1))
                {
                    cmd_str[cmd_idx++] = (char)c;
                    partial_line[partial_len++] = (char)c;
                    HAL_UART_Transmit(&huart2, &c, 1, 50);
                }else{//if user type so much char after !
                    parser_clear_partial_line();
                    partial_len = 0;
                    parser_state = PARSER_WAIT_START;
                }
            }
        }
    }
}
