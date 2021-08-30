/**
 ******************************************************************************
 * @file 	hp.h
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 ******************************************************************************
 */

#ifndef HP_HP_H_
#define HP_HP_H_

void HP_init(void);
void HP_process_test(void);
void HP_arriere(void);
void HP_detresse(void);
void HP_klaxon(void);
void HP_marche(void);
void HP_setTimer(uint32_t);
uint32_t HP_getTimer(void);

#endif /* HP_HP_H_ */
