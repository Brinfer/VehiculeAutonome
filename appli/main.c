/**
  ******************************************************************************
  * @file    main.c
  * @author  Gautier - Dufourmantelle
  * @date    27-January-2020
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f1xx_hal.h"
#include "stm32f1_uart.h"
#include "stm32f1_sys.h"
#include "stm32f1_gpio.h"
#include "macro_types.h"
#include "systick.h"

#include "led/led.h"
#include "hp/hp.h"
#include "moteur/moteur.h"
#include "capteur/capteur.h"

#define DELAY_COTE 3000	/** @def Temps maximale ou la voiture peut tourner, evite de tourner en rond (en ms)*/
#define DELAY_ARRIERE 5000 /** @def Temmps maximale ou la voiture peut reculer (en ms)*/

#define TEST 0  /** @def Variable indiquant si l'ont souhaite proceder aux testes des différents element de la voiture*/
#define MUSIC 1 /** @def Variable indiquant si l'on souhaite ou non la musique lorsque la voiture est en marche avant*/

typedef enum
{
	ARRET = 0,
	MARCHE,
	GAUCHE,
	DROITE,
	ARRIERE,
	INIT
} state_e; /** @enum Enumeration contenant les differents etats possiblde de la voiture*/

typedef struct
{
	uint8_t AVANT;
	uint8_t DROIT;
	uint8_t GAUCHE;
	uint8_t ARRIERE;
} capteur_s; /** @struct Structure regroupant les id des capteurs*/

static const capteur_s capteurID = (capteur_s){0, 1, 2, 3};
static volatile state_e etatVoiture = INIT;
static volatile bool_e on = FALSE;
static volatile uint32_t MAIN_timer = 0;

static void MAIN_process_ms(void);

/**
 * @brief Fonction permettant d'avoir un compte du temps passe
 */
static void MAIN_process_ms(void)
{
	MAIN_timer++;
}

int main(void)
{
	//Initialisation de la couche logicielle HAL (Hardware Abstraction Layer)
	//Cette ligne doit rester la premi�re �tape de la fonction main().
	HAL_Init();

	//Initialisation de l'UART2 � la vitesse de 115200 bauds/secondes (92kbits/s) PA2 : Tx  | PA3 : Rx.
	//Attention, les pins PA2 et PA3 ne sont pas reli�es jusqu'au connecteur de la Nucleo.
	//Ces broches sont redirig�es vers la sonde de d�bogage, la liaison UART �tant ensuite encapsul�e sur l'USB vers le PC de d�veloppement.
	UART_init(UART2_ID, 115200);

	//"Indique que les printf sortent vers le p�riph�rique UART2."
	SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	//On ajoute la fonction MAIN_process_ms � la liste des fonctions appel�es automatiquement chaque ms par la routine d'interruption du p�riph�rique SYSTICK
	Systick_add_callback_function(&MAIN_process_ms);

	MOTEUR_init();  //Initialisation des moteurs
	HP_init();		//Initialisation du Haut-Parleur
	CAPTEUR_init(); //Initialisation des capteurs
	LED_init();		//Initialisation de la LED RGB

#if TEST
	MAIN_timer = 0;
	Systick_add_callback_function(&MOTEUR_process_test); //Prend 4s avant de finir
	//Systick_add_callback_function(&HP_process_test);	 //Prend 4s avant de finir
	CAPTEUR_process_test();								 //Prend 4s avant de finir
	//Systick_add_callback_function(&LED_process_test);	//Prend 4s avant de finir

	while (MAIN_timer <= 5000)
	{ //Boucle d'attente, permettant de réaliser l'ensemble des test sans difficulté
		continue;
	}
	MAIN_timer = 0;
#endif

	while (1)
	{
		switch (etatVoiture)
		{
		case INIT:   //Cas au demarage de la voiture
		case MARCHE: //Cas ou la voiture est en marche avant
			if (!obstacle(capteurID.AVANT))
			{ //Absence obstacle
				if (!on)
				{
					marcheAvant();							   //Mise en marche des moteurs
					Systick_add_callback_function(&LED_avant); //Ajout dans la file des interruption de l'allumage de la LED
#if MUSIC
					Systick_add_callback_function(&HP_marche);
#endif
					on = TRUE; //Variable permettant de ne pas rallumer des moteurs d�j� allum�s
					etatVoiture = MARCHE;
				}
				if (MAIN_timer > 15000)
				{ //Permet d'eviter que la voiture aille tout le temps tout droit
					arret();
					on = FALSE;
					etatVoiture = DROITE;
				}
				break;
			}
			else
			{
				MAIN_timer = 0;
				arret();
				on = FALSE;
				bool_e routeLibere = FALSE;
#if MUSIC
				Systick_remove_callback_function(&HP_marche);
#endif
				Systick_add_callback_function(&HP_klaxon);
				while (MAIN_timer < 5000 && !routeLibere)
				{ //Boucle laissant 5s a l'operateur de deplacer l'obstacle devant la voiture
					if (!obstacle(capteurID.AVANT))
					{
						routeLibere = TRUE;
						Systick_remove_callback_function(&HP_klaxon);
					}
				}
				Systick_remove_callback_function(&LED_avant);
				if (routeLibere)
					break;
			}
			//Si obstacle se trouve devant la voiture, celle-ci regarde ensuite sur la droite immediatement
			/* no break */

		case DROITE:
			if (!obstacle(capteurID.DROIT))
			{ //Absence obstacle
				if (!on)
				{
					tourneDroite();
					Systick_add_callback_function(&LED_cote);
					on = TRUE;
					etatVoiture = DROITE;
					MAIN_timer = 0;
				}
				else if (MAIN_timer > DELAY_COTE)
					etatVoiture = INIT; //La voiture regardere de nouveau devant elle
				break;
			}
			on = FALSE;
			Systick_remove_callback_function(&LED_cote);
			//Si obstacle se trouve a droite de la voiture, celle-ci regarde ensuite sur la gauche immediatement
			/* no break */

		case GAUCHE:
			if (!obstacle(capteurID.GAUCHE))
			{ //Absence obstacle
				if (!on)
				{
					tourneGauche();
					Systick_add_callback_function(&LED_cote);
					on = TRUE;
					etatVoiture = GAUCHE;
					MAIN_timer = 0;
				}
				else if (MAIN_timer > DELAY_COTE)
					etatVoiture = INIT;
				break;
			}
			on = FALSE;
			Systick_remove_callback_function(&LED_cote);
			//Si obstacle se trouve a gauche de la voiture, celle-ci regarde ensuite a l'arriere immediatement
			/* no break */

		case ARRIERE:
			if (!obstacle(capteurID.ARRIERE))
			{ //Absence obstacle
				if (!on)
				{
					marcheArriere();
					Systick_add_callback_function(&LED_arriere);
					Systick_add_callback_function(&HP_arriere);
					on = TRUE;
					etatVoiture = ARRIERE;
					MAIN_timer = 0;
				}
				else if (MAIN_timer > DELAY_ARRIERE)
					etatVoiture = INIT;
				break;
			}
			on = FALSE;
			Systick_remove_callback_function(&LED_arriere);
			Systick_remove_callback_function(&HP_arriere);
			//Si obstacle se trouve derriere la voiture, celle-ci regarde s'arrete immediatement
			/* no break */

		default:
			if (!on)
			{
				arret(); //Arret des moteurs
				Systick_add_callback_function(&LED_detresse);
				Systick_add_callback_function(&HP_detresse);
				etatVoiture = ARRET;
				on = TRUE;
			}
		}
	}
}
