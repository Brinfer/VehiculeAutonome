/**
 ******************************************************************************
 * @file 	moteur.c
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 * @brief   Fonction associe aux moteurs
 ******************************************************************************
 */

#include "macro_types.h"
#include "stm32f1_motorDC.h"
#include "systick.h"
#include "config.h"
#include "moteur.h"

#define MOTEURD MOTOR1
#define MOTEURG MOTOR2

#define POWER_AVANT 100  /** @def Puissance des moteurs en marche avant (en %)*/
#define POWER_ARRIERE 65 /** @def Puissance des moteurs en marche arrierre (en %)*/
#define POWER_TOURNE 50  /** @def Puissance des moteurs en marche quand la voiture tourne (en %)*/

/**
 * @brief Fonction permettant d'initialiser nos deux moteurs
 */
void MOTEUR_init(void)
{
	MOTOR_init(2);
}
/**
 * @brief Fonction permettant de tester le fonctionnment des moteurs suivant une sequence :
 * 		- Avant
 * 		- Droite
 * 		- Gauche
 * 		- Arrierre
 * 		- Arret
 * @pre 	Il faut avoir active les test dans le main
 */
void MOTEUR_process_test(void)
{
	static bool_e launch = FALSE;
	static uint32_t timer = 0;

	timer++;

	if (!launch && timer < 2)
	{
		MOTOR_set_duty(POWER_AVANT, MOTEURD);
		MOTOR_set_duty(POWER_AVANT, MOTEURG);
		launch = TRUE;
	}
	else if (launch && timer >= 1000 && timer < 2000)
	{
		MOTOR_set_duty(-POWER_TOURNE, MOTEURG);
		MOTOR_set_duty(POWER_TOURNE, MOTEURD);
		launch = FALSE;
	}
	else if (!launch && timer >= 2000 && timer < 3000)
	{
		MOTOR_set_duty(-POWER_TOURNE, MOTEURD);
		MOTOR_set_duty(POWER_TOURNE, MOTEURG);
		launch = TRUE;
	}
	else if (launch && timer >= 3000 && timer < 4000)
	{
		MOTOR_set_duty(-POWER_ARRIERE, MOTEURG);
		MOTOR_set_duty(-POWER_ARRIERE, MOTEURD);
		launch = FALSE;
	}
	else if (timer >= 4000)
	{
		MOTOR_set_duty(0, MOTEURD);
		MOTOR_set_duty(0, MOTEURG);
		Systick_remove_callback_function(&MOTEUR_process_test);
	}
}

/**
 * @brief Fonction mettant les moteurs en marche avant
 */
void marcheAvant(void)
{
	MOTOR_set_duty(POWER_AVANT, MOTEURD);
	MOTOR_set_duty(POWER_AVANT, MOTEURG);
}

/**
 * @brief Fonction mettant les moteurs en marche arriere
 */
void marcheArriere(void)
{
	MOTOR_set_duty(-POWER_ARRIERE, MOTEURD);
	MOTOR_set_duty(-POWER_ARRIERE, MOTEURG);
}

/**
 * @brief Fonction mettant les moteurs en roue libre (non alimente)
 */
void arret(void)
{
	MOTOR_set_duty(0, MOTEURD);
	MOTOR_set_duty(0, MOTEURG);
}

/**
 * @brief Fonction mettant les moteurs en opposition pour tourner a droite
 */
void tourneDroite(void)
{
	MOTOR_set_duty(-POWER_TOURNE, MOTEURD);
	MOTOR_set_duty(POWER_TOURNE, MOTEURG);
}

/**
 * @brief Fonction mettant les moteurs en opposition pour tourner a gauche
 */
void tourneGauche(void)
{
	MOTOR_set_duty(POWER_TOURNE, MOTEURD);
	MOTOR_set_duty(-POWER_TOURNE, MOTEURG);
}
