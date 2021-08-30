/**
 ******************************************************************************
 * @file 	ledr.c
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 * @brief   Fonction associe a la led RVB
 ******************************************************************************
 */

#include "macro_types.h"
#include "stm32f1_gpio.h"
#include "systick.h"
#include "stm32f1_pwm.h"
#include "config.h"
#include "led.h"

#define PIN_R GPIO_PIN_15
#define PIN_B GPIO_PIN_12
#define PIN_V GPIO_PIN_11

#define GPIO_R GPIOA
#define GPIO_V GPIOA
#define GPIO_B GPIOA

static volatile uint32_t LED_timer = 0;
static void LED_timer_process(void);

/**
 * @brief Accesseur en lecture du timer de la led
 * @retval le timer de la led
 */
uint32_t LED_getTimer(void)
{
	return LED_timer;
}

/**
 * @brief Fonction permettant d'avoir un compte du temps passe
 */
static void LED_timer_process(void)
{
	LED_timer++;
}

/**
 * @brief Fonction permettant de changer la valeur du timer
 * @param newTimer : entier positif correspondant a la valeur du nouveau timer
 */
void LED_setTimer(uint32_t newTimer)
{
	LED_timer = newTimer;
}

/**
 * @brief Fonction permettant d'initialiser toutes les proches de la LED RGB et de mettre toute ces broches au niveau bas:
 * 			Rouge ==> 	A15
 * 			Vert ==>	A11
 * 			Bleu ==>	A12 
 */
void LED_init(void)
{
	BSP_GPIO_PinCfg(GPIO_R, PIN_R, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
	BSP_GPIO_PinCfg(GPIO_V, PIN_V, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
	BSP_GPIO_PinCfg(GPIO_B, PIN_B, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
	HAL_GPIO_WritePin(GPIO_B, PIN_B, 0);
	HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
	HAL_GPIO_WritePin(GPIO_V, PIN_V, 0);
	Systick_add_callback_function(&LED_timer_process); //Ajout du timer en interruption
}

/**
 * @brief 	Fonction permettant de tester le bon fonctionnement de la LED RGB,
 * 			celle-ci fera 4 cycles ou elle passera du bleu, au rouge, au vert puis au blanc
 * @pre 	Il faut avoir active les test dans le main
 */
void LED_process_test(void)
{
	static uint8_t comptCall = 0;
	static bool_e launch = FALSE;

	if (!launch && LED_timer < 10)
	{
		HAL_GPIO_WritePin(GPIO_B, PIN_B, 1);
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 0);
		launch = TRUE;
	}
	else if (launch && LED_timer >= 250 && LED_timer < 500)
	{
		HAL_GPIO_WritePin(GPIO_B, PIN_B, 0);
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 1);
		launch = FALSE;
	}
	else if (!launch && LED_timer >= 500 && LED_timer < 750)
	{
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 1);
		launch = TRUE;
	}
	else if (launch && LED_timer >= 750 && LED_timer < 1000)
	{
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 1);
		HAL_GPIO_WritePin(GPIO_B, PIN_B, 1);
		launch = FALSE;
	}
	else if (LED_timer >= 1000)
	{
		LED_setTimer(0);
		comptCall++;
	}

	if (comptCall >= 4)
	{
		HAL_GPIO_WritePin(GPIO_B, PIN_B, 0);
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 0);
		Systick_remove_callback_function(&LED_process_test);
	}
}

/**
 * @brief Fonction allument la LED afin d'indiquer que la voiture est bloque
 * 			celle-ci glignote en faisant SOS en morse
 */
void LED_detresse(void)
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
		if (!on && LED_timer >= pauseTimer)
		{
			HAL_GPIO_WritePin(GPIO_R, PIN_R, 1);
			on = TRUE;
			compt++;
			LED_setTimer(0);
		}
		else if (on && court && LED_timer > courtTimer && compt <= 3)
		{
			HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
			on = FALSE;
			LED_setTimer(0);
		}
		else if (on && !court && LED_timer > longTimer && compt <= 3)
		{
			HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
			on = FALSE;
			LED_setTimer(0);
		}
		else if (compt > 3)
		{
			compt = 1; //Il faut le mettre à 1 car quand on rentre dans cette boucle, la LED est allumé
			cycle++;
			court = !court;
			LED_setTimer(0);
		}
		/*On fait une fause entre chaque SOS*/
	}
	else if (LED_timer >= 5 * pauseTimer)
	{
		cycle = 0;
		compt = 0;
		court = TRUE;
		LED_setTimer(0);
	}
	else if (on)
	{
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
		on = FALSE;
	}
}
/**
 * @brief Fonction allument la LED afin d'indiquer que la voiture est en marche avant
 * 			la led clignote en vert
 */
void LED_avant(void)
{
	static bool_e on = FALSE;
	if (LED_timer < 500 && !on)
	{
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 1);
		on = TRUE;
		if (LED_timer < 10)
		{
			HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
			HAL_GPIO_WritePin(GPIO_B, PIN_B, 0);
		}
	}
	else if (LED_timer >= 500 && on)
	{
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 0);
		on = FALSE;
	}
	else if (LED_timer > 1000)
		LED_setTimer(0);
}
/**
 * @brief Fonction allument la LED afin d'indiquer que la voiture tourne
 * 			la led clignote en bleu
 */
void LED_cote(void)
{
	static bool_e on = FALSE;
	if (LED_timer < 500 && !on)
	{
		HAL_GPIO_WritePin(GPIO_B, PIN_B, 1);
		on = TRUE;
		if (LED_timer < 2)
		{
			HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
			HAL_GPIO_WritePin(GPIO_V, PIN_V, 0);
		}
	}
	else if (LED_timer >= 500 && on)
	{
		HAL_GPIO_WritePin(GPIO_B, PIN_B, 0);
		on = FALSE;
	}
	else if (LED_timer > 1000)
		LED_setTimer(0);
}
/**
 * @brief Fonction allument la LED afin d'indiquer que la voiture est en marche arriere
 * 			la led clignote en jaune (rouge + vert)
 */
void LED_arriere(void)
{
	static bool_e on = FALSE;
	if (LED_timer < 500 && !on)
	{
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 1);
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 1);
		on = TRUE;
		if (LED_timer < 2)
		{
			HAL_GPIO_WritePin(GPIO_B, PIN_B, 0);
		}
	}
	else if (LED_timer >= 500 && on)
	{
		HAL_GPIO_WritePin(GPIO_R, PIN_R, 0);
		HAL_GPIO_WritePin(GPIO_V, PIN_V, 0);
		on = FALSE;
	}
	else if (LED_timer > 1000)
		LED_setTimer(0);
}
