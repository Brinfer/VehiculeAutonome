/**
 ******************************************************************************
 * @file 	moteur.h
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 ******************************************************************************
 */

#ifndef MOTEUR_H_
#define MOTEUR_H_


void MOTEUR_process_test(void);
void marcheAvant(void);
void marcheArriere(void);
void arret(void);
void tourneDroite(void);
void tourneGauche(void);
void MOTEUR_init(void);

#endif /* MOTEUR_H_ */
