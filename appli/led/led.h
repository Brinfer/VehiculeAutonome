/**
 ******************************************************************************
 * @file 	led.c
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 ******************************************************************************
 */

#ifndef LED_H_
#define LED_H_

void LED_process_test(void);
void LED_init(void);
void LED_detresse(void);
void LED_avant(void);
void LED_cote(void);
void LED_arriere(void);
void LED_setTimer(uint32_t);
uint32_t LED_getTimer(void);

#endif /* LED_H_ */
