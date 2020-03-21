#ifndef CHOIX_CONFIG_H
#define CHOIX_CONFIG_H

/*---------------------------------------------
	Choisir entre le capteur de particule 
	SDS011 (valeur = 1) et PMS7003 (valeur = 2)
	
	exemple :
	
SDS011 	==>   #define CHOIX_CAPTEUR_PARTICULE			(1U) 
PMS7003 ==>	  #define CHOIX_CAPTEUR_PARTICULE			(2U)

Pas de capteur ==> #define CHOIX_CAPTEUR_PARTICULE		(0U)
----------------------------------------------*/

#define CHOIX_CAPTEUR_PARTICULE		(1U)

/*---------------------------------------------
Choisir entre le capteur de humidité/temperature
	DHT22 (valeur = 1) et BME280 (valeur = 2)
	
	exemple :

DHT22 	==>	  #define CHOIX_CAPTEUR_HT			(1U)
BME280	==>   #define CHOIX_CAPTEUR_HT			(2U)

Pas de capteur ==> #define CHOIX_CAPTEUR_HT		(0U)
----------------------------------------------*/

#define CHOIX_CAPTEUR_HT			(2U)

/*---------------------------------------------
	Choisir la période de mesure pour les 2 
	capteurs et la période d'envoi via Lora
		!! Temps en seconde !!
----------------------------------------------*/

// The frequency of transmission of datas	
#define PERIOD_LORA			(900U)
// The sensor measurement period
#define PERIOD_PARTICULE	(300U)
#define PERIOD_HT			(900U)

#endif /* CHOIX_CONFIG_H */

/********************************* PINMAP ******************************/
//	Tout les capteurs sont reliés au 5V / GND
//	
//		SDS011
//				TX = D2
//				RX = D8
//
//		PMS7003:
//				TX = D2
//				RX = D8
//				SET = D7
//   
//		DHT22:
//				DATA = D4
//
//		BME280: 
//				SDI = D14
//				SCK = D15
/***********************************************************************/
    
//luftdaten.info

//188mA
//repos 77mA

/* Grafana
Port: 8086

Juste pourcentage de la batterie
*/
