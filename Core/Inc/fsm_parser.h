/*
 * fsm_parser.h
 *
 *  Created on: Nov 18, 2026
 *      Author: trungvu
 */

#ifndef INC_FSM_PARSER_H_
#define INC_FSM_PARSER_H_

#include "main.h"
#include <stdint.h>

#include "main.h"
#include <stdint.h>

#define MAX_BUFFER_SIZE 30

/* Parsed command types */
#define CMD_NONE 0
#define CMD_RST  1
#define CMD_OK   2

/* Shared UART ring buffer variables (defined in main.c) */
extern uint8_t buffer[MAX_BUFFER_SIZE];
extern uint8_t index_buffer;
extern uint8_t buffer_flag;

/* Output variables from command_parser_fsm to uart_fsm */
extern uint8_t command_flag;
extern uint8_t command_data;

void command_parser_fsm(void);
void parser_clear_partial_line(void);
void parser_restore_partial_line(void);

#endif /* INC_FSM_PARSER_H_ */
