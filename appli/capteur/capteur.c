/**
 ******************************************************************************
 * @file 	capteur.c
 * @date    27-January-2020
 * @author  Gautier - Dufourmantelle
 * @brief   Fonction associe aux capteurs
 * @see 	void HCSR04_demo_state_machine(void) dans le fichier HCSR04.c
 ******************************************************************************
 */

#include "macro_types.h"
#include "stm32f1_uart.h"
#include "systick.h"
#include "HC-SR04/HCSR04.h"
#include "capteur.h"

#define DISTANCE_OBSTACLE 1500 /** @def Distance maximale a laquelle peut se trouver un obstacle devant un capteur*/

typedef struct
{
	uint16_t PIN_TRIG;
	GPIO_TypeDef *GPIO_TRIG;
	uint16_t PIN_ECHO;
	GPIO_TypeDef *GPIO_ECHO;
	uint8_t ID;
} capteur_t; /** @struct Structure regroupant les infortions liees aux capteurs*/

static const capteur_t capteurAvant = (capteur_t){GPIO_PIN_4, GPIOA, GPIO_PIN_8, GPIOB, 0};
static const capteur_t capteurDroite = (capteur_t){GPIO_PIN_5, GPIOA, GPIO_PIN_9, GPIOB, 0};
static const capteur_t capteurGauche = (capteur_t){GPIO_PIN_6, GPIOA, GPIO_PIN_10, GPIOB, 0};
static const capteur_t capteurArriere = (capteur_t){GPIO_PIN_7, GPIOA, GPIO_PIN_11, GPIOB, 0};

static uint16_t launch_measure(uint8_t);

/**
 * @brief Fonction permettant de mesurer la distance à laquelle se trouve un éventuel obstacle devant le capteur
 * 			dont l'identifiant est passe en parametre
 * @param id_sensor : identifiant du capteur
 * @retval la distance en mm a laquelle se trouve l'obstacle,  
 * @retval 0xFFFF s'il n'y a pas d'obstacle ou qu'il est trop loin
 * @note Fonction copie
 * @see void HCSR04_demo_state_machine(void) dans le fichier HCSR04.c
 * @author Nirgal
 */
static uint16_t launch_measure(uint8_t id_sensor)
{
	typedef enum
	{
		LAUNCH_MEASURE,
		RUN,
		WAIT_DURING_MEASURE,
		WAIT_BEFORE_NEXT_MEASURE
	} state_e;

	static state_e state = LAUNCH_MEASURE;
	static uint32_t tlocal;
	uint16_t distance = 65535; //valeur max sur 16 bits, si on retourne cette valeur c'est que la meusure n'est pas faite

	HCSR04_process_main();

	switch (state)
	{
	case LAUNCH_MEASURE:
		HCSR04_run_measure(id_sensor);
		tlocal = HAL_GetTick();
		state = WAIT_DURING_MEASURE;
		break;
	case WAIT_DURING_MEASURE:
		switch (HCSR04_get_value(id_sensor, &distance))
		{
		case HAL_BUSY:
			//rien � faire... on attend...
			break;
		case HAL_OK:
			printf("sensor %d - distance : %d\n", id_sensor, distance);
			state = WAIT_BEFORE_NEXT_MEASURE;
			break;
		case HAL_ERROR:
			printf("sensor %d - erreur ou mesure non lanc�e\n", id_sensor);
			state = WAIT_BEFORE_NEXT_MEASURE;
			break;

		case HAL_TIMEOUT:
			printf("sensor %d - timeout\n", id_sensor);
			state = WAIT_BEFORE_NEXT_MEASURE;
			break;
		}
		break;
	case WAIT_BEFORE_NEXT_MEASURE:
		if (HAL_GetTick() > tlocal + 100)
			state = LAUNCH_MEASURE;
		break;
	default:
		break;
	}
	return distance;
}

/**
 * @brief Fonction permettant d'initialiser les 4 capteurs
 * @pre   Chaques capteurs doivent avoir un id different et etre associe a des broches differentes
 * @post  Un message est affiche dans le cas ou une erreur est survenue, sinon on affiche la reussite de l'initialisation
 */
void CAPTEUR_init(void)
{
	HAL_StatusTypeDef ret;
	ret = HCSR04_add(&capteurAvant.ID, capteurAvant.GPIO_TRIG, capteurAvant.PIN_TRIG, capteurAvant.GPIO_ECHO, capteurAvant.PIN_ECHO);
	if (ret != HAL_OK)
		printf("Erreur ajout capteur avant");
	else
	{
		ret = HCSR04_add(&capteurDroite.ID, capteurDroite.GPIO_TRIG, capteurDroite.PIN_TRIG, capteurDroite.GPIO_ECHO, capteurDroite.PIN_ECHO);
		if (ret != HAL_OK)
			printf("Erreur ajout capteur droit");
		else
		{
			ret = HCSR04_add(&capteurGauche.ID, capteurGauche.GPIO_TRIG, capteurGauche.PIN_TRIG, capteurGauche.GPIO_ECHO, capteurGauche.PIN_ECHO);
			if (ret != HAL_OK)
				printf("Erreur ajout capteur gauche");
			else
			{
				ret = HCSR04_add(&capteurArriere.ID, capteurArriere.GPIO_TRIG, capteurArriere.PIN_TRIG, capteurArriere.GPIO_ECHO, capteurArriere.PIN_ECHO);
				if (ret != HAL_OK)
					printf("Erreur ajout capteur arriere");
				else
					printf("Tout les capteurs ont été ajouté avec succé");
			}
		}
	}
}

/**
 * @brief 	Fonction permettant de tester le bon fonctionnement des capteurs,
 * 			ceux-ci seront allume pendant 4s
 * @pre 	Il faut avoir active les test dans le main
 */
void CAPTEUR_process_test(void)
{
	static uint32_t t_capteur = 0;

	while (t_capteur < 4000)
	{
		t_capteur++;
		launch_measure(capteurGauche.ID);
		launch_measure(capteurAvant.ID);
		launch_measure(capteurDroite.ID);
		launch_measure(capteurArriere.ID);
	}
}

/**
 * @brief Retourne un booleen, si un obstacle à moins de 10cm
 * @param id_sensor : identifiant du capteur
 * @retval FALSE si on mesure un obstacle a 0mm ou au dela de la limite fixe
 * @retval TRUE si on mesure un obstacle a plus de 0mm et en dessous de la limite fixe
 */
bool_e obstacle(uint8_t id)
{
	uint16_t mesure = launch_measure(id);
	bool_e ret = FALSE;
	if (mesure < DISTANCE_OBSTACLE && mesure != 0)
		ret = TRUE;
	return ret;
}
