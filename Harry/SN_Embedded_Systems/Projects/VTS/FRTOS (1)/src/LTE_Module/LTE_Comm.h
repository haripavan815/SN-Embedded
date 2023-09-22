/* LTE Communication */
#ifndef LTE_COMM_H_
#define LTE_COMM_H_

#include <asf.h>
#include <conf_board.h>
Bool processAT_Command(uint8_t *cmd );
void task_LTE_Parser(void *pvParameters);
void task_LTE_Gps_Parser(void *pvParameters);
void task_LTE_Commander(void *pvParameters);
 
#endif /* LTE_COMM_H_ */  