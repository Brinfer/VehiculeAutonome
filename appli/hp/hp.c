/**
 ******************************************************************************
 * @file 	hp.c
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 * @brief   Fonction associe au Haut parleur
 ******************************************************************************
 */

#include "stm32f1_pwm.h"
#include "macro_types.h"
#include "systick.h"
#include "hp.h"
#include "config.h"

#define POWER 50 /** @def amplitude en % pour la generation du son*/

#define PIN_HP GPIO_PIN_6 //Broche PB6
#define GPIO_HP GPIOB

#define TIMER TIMER4_ID
#define CHANNEL TIM_CHANNEL_1

#define BEEPPER ((uint32_t)60000)  /** @def periode en µs du son pour la marche arriere*/
#define DETRESSE ((uint32_t)10000) /** @def periode en µs du son quand la voiture est coince*/
#define KLAXON ((uint32_t)65535)   /** @def periode en µs du son du klaxon*/

#define DO ((uint32_t)3817)  /** @def periode en µs de la note DO*/
#define RE ((uint32_t)3401)  /** @def periode en µs de la note RE*/
#define MI ((uint32_t)3030)  /** @def periode en µs de la note MI*/
#define FA ((uint32_t)2865)  /** @def periode en µs de la note FA*/
#define SOL ((uint32_t)2551) /** @def periode en µs de la note SOL*/
#define LA ((uint32_t)2273)  /** @def periode en µs de la note LA*/
#define SI ((uint32_t)2024)  /** @def periode en µs de la note SI*/

static const uint16_t melodie[] = {
	379, 379, 0, 379,
	0, 477, 379, 0,
	318, 0, 0, 0,
	637, 0, 0, 0,

	477, 0, 0, 637,
	0, 0, 658, 0,
	0, 568, 0, 506,
	0, 536, 568, 0,

	637, 379, 318,
	284, 0, 357, 318,
	0, 379, 0, 477,
	425, 506, 0, 0,

	477, 0, 0, 637,
	0, 0, 658, 0,
	0, 568, 0, 506,
	0, 536, 568, 0,

	637, 379, 318,
	284, 0, 357, 318,
	0, 379, 0, 477,
	425, 506, 0, 0};

static const uint8_t tempo[] = {
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,

	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,

	111,
	111,
	111,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,

	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,

	111,
	111,
	111,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
	83,
};

static volatile uint32_t HP_timer = 0;

static void HP_timer_process(void);

/**
 * @brief Accesseur en lecture du timer du HP
 * @retval le timer du HP
 */
uint32_t HP_getTimer(void)
{
	return HP_timer;
}

/**
 * @brief Fonction permettant d'avoir un compte du temps passe
 * @pre 	Il faut avoir active les test dans le main
 */
void HP_timer_process(void)
{
	HP_timer++;
}

/**
 * @brief Fonction permettant de changer la valeur du timer
 * @param newTimer : entier positif correspondant a la valeur du nouveau timer
 */
void HP_setTimer(uint32_t newTimer)
{
	HP_timer = newTimer;
}

/**
 * @brief Fonction forcant la mise au niveau bas de la broche pilotant le HP
 */
void HP_init(void)
{
	Systick_add_callback_function(&HP_timer_process); //Ajout du timer en interruption
	PWM_run(TIMER, CHANNEL, FALSE, DO, 0, FALSE);
}

/**
 * @brief 	Fonction permettant de tester le bon fonctionnement de la LED RGB,
 * 			celle-ci fera DO RE MI FA SOL LA SI DO
 */
void HP_process_test(void)
{
	static bool_e on = FALSE;

	if (!on && HP_timer < 10)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, DO, POWER);
		on = TRUE;
	}
	else if (on && HP_timer >= 500 && HP_timer < 1000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, RE, POWER);
		on = FALSE;
	}
	else if (!on && HP_timer >= 1000 && HP_timer < 1500)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, MI, POWER);
		on = TRUE;
	}
	else if (on && HP_timer >= 1500 && HP_timer < 2000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, FA, POWER);
		on = FALSE;
	}
	else if (!on && HP_timer >= 2000 && HP_timer < 2500)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, SOL, POWER);
		on = TRUE;
	}
	else if (on && HP_timer >= 2500 && HP_timer < 3000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, LA, POWER);
		on = FALSE;
	}
	else if (!on && HP_timer >= 3000 && HP_timer < 3500)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, SI, POWER);
		on = TRUE;
	}
	else if (on && HP_timer >= 3500 && HP_timer < 4000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, DO, POWER);
		on = FALSE;
	}
	else if (HP_timer >= 4000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, DO, 0);
		HP_setTimer(0);
		Systick_remove_callback_function(&HP_process_test);
	}
}
/**
 * @brief Fonction permettant d'indiquer que la voiture fait une marche arriere
 * 			le HP bip
 */
void HP_arriere(void)
{
	static bool_e on = FALSE;

	if (!on && HP_timer < 10)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, BEEPPER, POWER);
		on = TRUE;
	}
	else if (on && HP_timer > 1000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, BEEPPER, 0);
		on = FALSE;
	}
	else if (HP_timer > 1250)
	{
		HP_setTimer(0);
	}
}

/**
 * @brief Fonction allument le HP afin d'indiquer que la voiture est bloque
 * 			celui-ci fait un SOS en morse
 */
void HP_detresse(void)
{
	static bool_e on = FALSE;
	static bool_e court = TRUE;
	static uint8_t compt = 0;
	static uint8_t cycle = 0;
	const uint16_t courtTimer = 250;
	const uint16_t longTimer = 500;
	const uint16_t pauseTimer = 250;

	if (cycle < 3)
	{
		if (!on && HP_timer >= pauseTimer)
		{
			PWM_run(TIMER, CHANNEL, FALSE, DETRESSE, POWER, FALSE);
			on = TRUE;
			compt++;
			HP_setTimer(0);
		}
		else if (on && court && HP_timer > courtTimer && compt <= 3)
		{
			PWM_run(TIMER, CHANNEL, FALSE, DETRESSE, 0, FALSE);
			on = FALSE;
			HP_setTimer(0);
		}
		else if (on && !court && HP_timer > longTimer && compt <= 3)
		{
			PWM_run(TIMER, CHANNEL, FALSE, DETRESSE, 0, FALSE);
			on = FALSE;
			HP_setTimer(0);
		}
		else if (compt > 3)
		{
			compt = 1; //Il faut le mettre à 1 car quand on rentre dans cette boucle, le HP est allumé
			cycle++;
			court = !court;
			HP_setTimer(0);
		}
	}
	else if (HP_timer >= 5 * pauseTimer)
	{
		cycle = 0;
		compt = 0;
		court = TRUE;
		HP_setTimer(0);
	}
	else if (on)
	{
		PWM_run(TIMER, CHANNEL, FALSE, DETRESSE, 0, FALSE);
		on = FALSE;
	}
}

/**
 * @brief Fonction faisant klaxonner la voiture
 */
void HP_klaxon(void)
{
	static bool_e on = FALSE;

	if (!on && HP_timer < 10)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, KLAXON, POWER);
		on = TRUE;
	}
	else if (on && HP_timer > 750 && HP_timer < 1000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, KLAXON, 0);
		on = FALSE;
	}
	else if (!on && HP_timer > 1000 && HP_timer < 2000)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, KLAXON, POWER);
		on = TRUE;
	}
	else if (on && HP_timer > 2000 && HP_timer < 2500)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, KLAXON, 0);
		on = FALSE;
	}
	else if (!on && HP_timer > 2500 && HP_timer < 4250)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, KLAXON, POWER);
		on = TRUE;
	}
	else if (on && HP_timer > 4250)
	{
		PWM_set_period_and_duty(TIMER, CHANNEL, KLAXON, 0);
		on = FALSE;
		Systick_remove_callback_function(&HP_klaxon);
	}
}

/**
 * @brief Fonction lançant la musique Mario losque que la voiture est en marche
 * @note  Le sons est horrible et agassant, par defaut la fonction n'est pas appele
 * @pre   Il faut avoir active la music dans le main
 */
void HP_marche(void)
{
	static bool_e on = FALSE;
	static uint8_t note = 0;

	if (melodie[note] != 0)
		PWM_run(TIMER, CHANNEL, FALSE, melodie[note], POWER, FALSE);
	else
		PWM_run(TIMER, CHANNEL, FALSE, DO, 0, FALSE); //si aucune melodie ne doit etre joue on met une periode quelconque mais avec une amplitude nulle

	if (HP_timer > tempo[note])
	{
		if (note < (sizeof(melodie) / sizeof(uint16_t))) //sizeof retourne litterallement la taille du tableau, soit le nombre de bit qu'il occupe et non le nombre d'element
			note++;
		else
			note = 0;
		HP_setTimer(0);
	}
}
