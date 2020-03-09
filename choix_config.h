#ifndef CHOIX_CONFIG_H
#define CHOIX_CONFIG_H

/*---------------------------------------------
	Choisir entre le capteur de particule 
	SDS011 (valeur = 1) et PMS7003 (valeur = 2)
	
	exemple :
	
SDS011 	==>   #define CHOIX_CAPTEUR_PARTICULE		(1U) 
PMS7003 ==>	  #define CHOIX_CAPTEUR_PARTICULE		(2U)
----------------------------------------------*/

#define CHOIX_CAPTEUR_PARTICULE		(1U)

/*---------------------------------------------
Choisir entre le capteur de humidité/temperature
	DHT22 (valeur = 1) et BME280 (valeur = 2)
	
	exemple :

DHT22 	==>	  #define CHOIX_CAPTEUR_HT				(1U)
BME280	==>   #define CHOIX_CAPTEUR_HT				(2U)
----------------------------------------------*/

#define CHOIX_CAPTEUR_HT			(1U)

/*---------------------------------------------
	Choisir la période de mesure pour les 2 
	capteurs et la période d'envoi via Lora
		!! Temps en seconde !!
----------------------------------------------*/
	
#define PERIOD_LORA			(60U)//15 à min
#define PERIOD_PARTICULE	(60U)
#define PERIOD_HT			(60U)

#endif /* CHOIX_CONFIG_H */

//luftdaten.info
/* 1 info toutes les 15 minutes */
