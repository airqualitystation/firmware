/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example demonstrating the use of LoRaWAN with RIOT
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *				Lucas TREVELOT
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include "xtimer.h"

//Capteur particule 
#include "sds011.h"
#include "sds011_params.h"
#include "pms7003.h"
#include "pms7003_params.h"

//Capteur temperature et humidité
#include "dht.h" 
#include "dht_params.h"
#include "bmx280.h"
#include "bmx280_params.h"

#include "msg.h"
#include "thread.h"
#include "fmt.h"

#include "periph/rtc.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

#include "cayenne_lpp.h"

#include "choix_config.h"

/********************************* Variable globale ***********************************/

#define RECV_MSG_QUEUE                   (4U)
static msg_t _recv_queue[RECV_MSG_QUEUE];
static char _recv_stack[THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t sender_pid;
static char sender_stack[THREAD_STACKSIZE_MAIN / 2];

#if CHOIX_CAPTEUR_PARTICULE == 0
//Pas de capteur de particule
#else
#define PARTICULE_MSG_QUEUE                   (4U)
static kernel_pid_t particule_pid;
static char particule_stack[THREAD_STACKSIZE_DEFAULT];
#endif

#if CHOIX_CAPTEUR_HT == 0
//Pas de capteur de temperature/humidite
#else
#define HT_MSG_QUEUE                   (4U)
static kernel_pid_t ht_pid;
static char ht_stack[THREAD_STACKSIZE_DEFAULT];
#endif

/* Messages are sent every 1000s to respect the duty cycle on each channel */
//#define PERIOD              (120U)
#define PUT_TO_QUERY_MODE_RETRIES                (3U)
#define PUT_TO_QUERY_MODE_RETRY_TIMEOUT_MS       (100U)

semtech_loramac_t loramac;

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

#if CHOIX_CAPTEUR_PARTICULE == 1 
static sds011_t dev_sds011;
#elif CHOIX_CAPTEUR_PARTICULE == 2
static pms7003_t dev_pms7003;
#elif CHOIX_CAPTEUR_PARTICULE == 0
//Pas de capteur de particule
#endif

#if CHOIX_CAPTEUR_HT == 1
static dht_t dev_dht;
#elif CHOIX_CAPTEUR_HT == 2
static bmx280_t dev_bme;
#elif CHOIX_CAPTEUR_HT == 0
//Pas de capteur de temperature/humidite
#endif

static cayenne_lpp_t lpp;

uint16_t Lora_PERIOD=PERIOD_LORA; // Modifiable dans le fichier choix_capteur.h
uint16_t ht_PERIOD=PERIOD_HT; // Modifiable dans le fichier choix_capteur.h
uint16_t particule_PERIOD=PERIOD_PARTICULE; // Modifiable dans le fichier choix_capteur.h
uint16_t cnt_Lora_PERIOD=0; //Stock le temps en seconde et se remet à 0 si cnt_Lora_PERIOD = Lora_PERIOD
uint16_t cnt_ht_PERIOD=0; //Pareil que cnt_Lora_PERIOD mais pour les capteur ht
uint16_t cnt_particule_PERIOD=0; //Pareil que cnt_Lora_PERIOD mais pour le capteur particule

uint16_t PERIOD=0;/* Stock la valeur de la plus petite période entre le capteur ht et le capteur particule, donc PERIOD = PERIOD_HT ou PERIOD_PARTICULE ou la valeur absolue de la différence entre PERIOD_HT et PERIOD_PARTICULE si < PERIOD minimale*/

float cnt_ht=0.00; //compte le nombre de fois que le traitement du capteur ht est effectué pendant la durée de 1 Lora_PERIOD
float cnt_particule=0.00; // pareil que cnt_ht mais pour le sds011 ou le pms7003

float tot_temp=0.00; //Stock la somme des mesures pour la temperature
float tot_hum=0.00; //Stock la somme des mesures pour l'humidité
float tot_pm2_5=0.00; //Stock la somme des mesures pour pm2.5
float tot_pm10=0.00; //Stock la somme des mesures pour pm10
#if CHOIX_CAPTEUR_PARTICULE == 2
float tot_pm1=0.00; //Stock la somme des mesures pour pm1, uniquement pour le pms7003
#endif

/*********************************** Prototype ****************************************/

#if CHOIX_CAPTEUR_PARTICULE == 1
static void Traitement_SDS011(void);
static void _print_measurement_sds011(float data_pm10,float data_pm2_5);
void measure_cb(sds011_data_t *data, void *ctx);
#elif CHOIX_CAPTEUR_PARTICULE == 2
static void Traitement_PMS7003(void);
static void _print_measurement_pms7003(float data_pm10,float data_pm2_5,float data_pm1);
void measure_cb_pm_standard(pms7003_data_t *data, void *ctx);
#endif

#if CHOIX_CAPTEUR_HT == 1
static void Traitement_DHT22(void);
#elif CHOIX_CAPTEUR_HT == 2
static void Traitement_BME280(void);
#endif

static void rtc_cb(void *arg);
static void _prepare_next_alarm(void);
static void _send_message(void);
static void *sender(void *arg);
static void *_recv(void *arg);

#if CHOIX_CAPTEUR_PARTICULE == 0
//Pas de capteur de particule
#else
static void *_particule(void *arg);
#endif

#if CHOIX_CAPTEUR_HT == 0
//Pas de capteur de temperature/humidite
#else
static void *_ht(void *arg);
#endif

/**************************************************************************************/

int main(void)
{   
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
    //
    //		Alim panneau solaire:
    //
    //				Orange = Vin
    //				Jaune = GND
    /***********************************************************************/
        
    /* initialize the driver */
      
    #if CHOIX_CAPTEUR_HT == 1 /* Initialize DHT22 */
   	if (dht_init(&dev_dht, &dht_params[0]) == DHT_OK) {
        puts("DHT22 : init [OK]");
    }
    else {
        puts("DHT22 : init [ERROR]");
        return -1;
    }
    #elif CHOIX_CAPTEUR_HT == 2 /* Initialize BME280 */
   	switch (bmx280_init(&dev_bme, &bmx280_params[0])) 
   	{
        case BMX280_ERR_BUS:
            puts("BME280 : init [Error] Something went wrong when using the I2C bus");
        return 1;
        
        case BMX280_ERR_NODEV:
            puts("BME280 : init [Error] Unable to communicate with any BMX280 device");
        return 1;
        
        default: /* all good -> do nothing */
            puts("BME280 : init [OK]");
        break;
    }
   	#elif CHOIX_CAPTEUR_HT == 0
   	//Pas de capteur de temperature/humidite
    #else /* ERROR */
    	puts("[ERROR] : Choix capteur HT ==> main");
    	return -1;
    #endif
   
    #if CHOIX_CAPTEUR_PARTICULE == 1 /* Initialize SDS011 */
    if (sds011_init(&dev_sds011, &sds011_params[0]) == SDS011_OK) { //Init sur UART2
       	puts("SDS011 : init [OK]");
    }
    else {
       	puts("SDS011 : init [ERROR]");
        return -1;
    }
    #elif CHOIX_CAPTEUR_PARTICULE == 2 /* Initialize PMS7003 */
   	if(pms7003_init(&dev_pms7003, &pms7003_params[0])>=0){
   		puts("PMS7003 : init [OK]");
   	}
   	else {
   		puts("PMS7003 : init [ERROR]");
   		return -1;
   	}
   	#elif CHOIX_CAPTEUR_PARTICULE == 0
   	//Pas de capteur de particule
    #else /* ERROR */
    	puts("[ERROR] : Choix capteur particule ==> main");
    	return -1;
    #endif
    
    puts("LoRaWAN Class A low-power application");
    puts("=====================================");

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, DEVEUI);
    fmt_hex_bytes(appeui, APPEUI);
    fmt_hex_bytes(appkey, APPKEY);

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    while(semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
    }
    puts("Join procedure succeeded");
    
    /* start the recv thread */
    thread_create(_recv_stack, sizeof(_recv_stack),THREAD_PRIORITY_MAIN - 1, 0, _recv, NULL, "recv thread");

    /* start the sender thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),THREAD_PRIORITY_MAIN - 1, 0, sender, NULL, "sender");
 	
 	#if CHOIX_CAPTEUR_PARTICULE == 0
 	//Pas de capteur de particule
 	#else 
    /* start the particule thread */
    particule_pid = thread_create(particule_stack, sizeof(particule_stack),THREAD_PRIORITY_MAIN - 1, 0, _particule, NULL, "capteur_particule");
    #endif
    
    #if CHOIX_CAPTEUR_HT == 0
    //Pas de capteur de temperature/humidite
    #else
    /* start the ht thread */
    ht_pid = thread_create(ht_stack, sizeof(ht_stack),THREAD_PRIORITY_MAIN - 1, 0, _ht, NULL, "capteur_ht");
    #endif

    /* trigger the first send */
    msg_t msg;
    #if CHOIX_CAPTEUR_PARTICULE == 0
 	//Pas de capteur de particule
    #else
    msg_send(&msg, particule_pid);//Envoi un message au thread particule
    #endif
    
    #if CHOIX_CAPTEUR_HT == 0
    //Pas de capteur de temperature/humidite
    #else
    msg_send(&msg, ht_pid);//Envoi un message au thread ht
    #endif
    
    return 0;
}




/*********************************** Functions ***************************************/

#if CHOIX_CAPTEUR_PARTICULE == 1
void measure_cb(sds011_data_t *data, void *ctx)
{
	/* Fonction d'interruption de l'UART
	* Retourne les valeurs du capteur particule
	*/
	
    msg_t msg = { .content.value = (((uint32_t)data->pm_10) << 16 | data->pm_2_5) };
    kernel_pid_t target_pid = (int)ctx;
    msg_send(&msg, target_pid);
}

static void _print_measurement_sds011(float data_pm10,float data_pm2_5)
{
    /************************ Affichage (optionnel) *************************/
    //Formule donnée dans la datasheet du capteur
    uint16_t pm10_ug_int = data_pm10 / 10;
    uint16_t pm10_ug_dec = data_pm10 - 10 * pm10_ug_int;
    uint16_t pm2_5_ug_int = data_pm2_5 / 10;
    uint16_t pm2_5_ug_dec = data_pm2_5 - 10 * pm2_5_ug_int;
        
    //Affichage des données dans le terminal
    printf("==> PM2.5: %d.%0d ug/m^3\n==> PM10: %d.%0d ug/m^3\n",pm2_5_ug_int, pm2_5_ug_dec,pm10_ug_int, pm10_ug_dec);
    /************************************************************************/
}

static void Traitement_SDS011(void)
{
	/* Fonction qui met le capteur SDS011 en marche (MODE_WORK) récupère les données pm10 et 		* pm2.5 à chaque interruption. Il effectue d'abord un cycle de 5 mesure pour laisser le 
	* temps au ventilateur de tourner, puis une fois terminer le capteur fait 10 nouvelles
	* mesures qu'il enregistre et retourne la moyenne des 10 dernières mesures dans les 
	* variables globales tot_pm2_5 et tot_pm10 puis s'endort (MODE_SLEEP)
	*/ 
	
	sds011_data_t data;
	msg_t sds011_msg;
	float moy_pm10=0.00;
	float moy_pm2_5=0.00;
	
	// sds011_register_callback initialise l'UART sur une fonction d'interruption (ici measure_cb)
	sds011_register_callback(&dev_sds011, measure_cb, (void*)(int)thread_getpid());
    
    //Allume le sds si il est sleep
	while(sds011_set_working_mode(&dev_sds011, SDS011_WMODE_WORK) != SDS011_OK);
    
    //mode ACTIVE permet d'avoir une valeur a chaque interruption
    while (sds011_set_reporting_mode(&dev_sds011, SDS011_RMODE_ACTIVE) != SDS011_OK) 
    {
       	xtimer_usleep(PUT_TO_QUERY_MODE_RETRY_TIMEOUT_MS * 1000);
       	//puts("[RETRY]");
    }
    
    //puts("[MESSAGE]");
    /* Fait 5 mesure pour lancer le ventilateur */
    for(unsigned msg_cnt = 0; msg_cnt < 5; msg_cnt++){
		msg_receive(&sds011_msg);//msg_send par la fonction measure_cb quand il a recu une valeur
		data.pm_10 = sds011_msg.content.value >> 16;
		data.pm_2_5 = sds011_msg.content.value & 0xFFFF;
    }
    /* enregistre 10 autres mesure mtn que le ventilateur a bien tourner */
    for(unsigned msg_cnt = 0; msg_cnt < 10; msg_cnt++){
		msg_receive(&sds011_msg);//msg_send par la fonction measure_cb quand il a recu une valeur
		data.pm_10 = sds011_msg.content.value >> 16;
		moy_pm10+=data.pm_10;//Stock les valeurs de pm10
		data.pm_2_5 = sds011_msg.content.value & 0xFFFF;
		moy_pm2_5+=data.pm_2_5;//Stock les valeurs de pm2.5
    }
    //puts("[FIN MESSAGE]");
	
	/* moyenne des 10 dernieres mesure */
	moy_pm10=moy_pm10/10.00; //divisé par 10 car 10 mesures
	moy_pm2_5=moy_pm2_5/10.00; //divisé par 10 car 10 mesures
	tot_pm10 += (moy_pm10/10.00); // divisé par 10 pour avoir un chiffre a virgule
    tot_pm2_5 += (moy_pm2_5/10.00); // divisé par 10 pour avoir un chiffre a virgule
	
    /* unregister callback */
    sds011_register_callback(&dev_sds011, NULL, NULL);
    
    //met en veille le capteur
    while(sds011_set_working_mode(&dev_sds011, SDS011_WMODE_SLEEP) != SDS011_OK);
       
    _print_measurement_sds011(moy_pm10,moy_pm2_5);//Affichage dans le terminal
}
#elif CHOIX_CAPTEUR_PARTICULE == 2
void measure_cb_pm_standard(pms7003_data_t *data, void *ctx)
{
    msg_t msg1 = { .content.value = (((uint32_t)data->pm_10) << 16 | data->pm_2_5) };
    msg_t msg2 = { .content.value = (((uint32_t)data->pm_1 )& 0xFFFF)};

    kernel_pid_t target_pid = (int)ctx;
    msg_send(&msg1, target_pid);
    msg_send(&msg2, target_pid);
}

static void _print_measurement_pms7003(float data_pm10,float data_pm2_5,float data_pm1)
{
    /************************ Affichage (optionnel) *************************/
    //Formule donnée dans la datasheet du capteur
    uint16_t pm10_ug_int = data_pm10 / 10;
    uint16_t pm10_ug_dec = data_pm10 - 10 * pm10_ug_int;
    uint16_t pm2_5_ug_int = data_pm2_5 / 10;
    uint16_t pm2_5_ug_dec = data_pm2_5 - 10 * pm2_5_ug_int;
    uint16_t pm1_ug_int = data_pm1 / 10;
    uint16_t pm1_ug_dec = data_pm1 - 10 * pm1_ug_int;
        
    //Affichage des données dans le terminal
    printf("==> PM1: %d.%0d ug/m^3\n==> PM2.5: %d.%0d ug/m^3\n==> PM10: %d.%0d ug/m^3\n",pm1_ug_int,pm1_ug_dec,pm2_5_ug_int, pm2_5_ug_dec,pm10_ug_int, pm10_ug_dec);
    /************************************************************************/
}

static void Traitement_PMS7003(void)
{
	/* Fonction qui met le capteur PMS7003 en marche, récupère les données pm10, pm2.5 et pm1
	* à chaque interruption. Il effectue d'abord un cycle de 5 mesure pour laisser le 
	* temps au ventilateur de tourner, puis une fois terminer le capteur fait 10 nouvelles
	* mesures qu'il enregistre et retourne la moyenne des 10 dernières mesures dans les 
	* variables globales tot_pm10, tot_pm2_5 et tot_pm1 puis s'endort (MODE_SLEEP)
	*/ 
	
	msg_t msg1, msg2;
	pms7003_data_t data;
	float moy_pm10=0.00;
	float moy_pm2_5=0.00;
	float moy_pm1=0.00;
	
	// In order to be sure that the device is not sleep 
	// Soft because we use the uart interface to send cmd at the device
	// You can use the hardware by connecting the coresponding pin to your board <WAKEUP_HARD>
    WAKEUP_SOFT(&dev_pms7003); 

	// The active mode is better accurate than passive
	// The active mode need a callback register procedure.
	// -> If you want use the passive mode, then you need to creat your own RX_CALLBACK and 	attached to the device
  	// For more details of the implementation of rx_callback for the passive mode, see <pms7003.c> in the driver folder.
	// In the passive mode you need to send read_passive command to obtain just one measure...
    ACTIVE_MODE(&dev_pms7003); 
    
	// Register the corresponding callback <measure_cb_pm_standard> for the measurement.
	// If you want unregister the callback use this :
    // ->  pms7003_register_callback(&dev,NULL,NULL);
	pms7003_register_callback(&dev_pms7003,measure_cb_pm_standard,(void*)(int)thread_getpid());
    
    for(unsigned msg_cnt = 0; msg_cnt < 5; msg_cnt++){
        msg_receive(&msg1);
        data.pm_10 =  msg1.content.value >> 16;
        data.pm_2_5 = msg1.content.value & 0xFFFF;
        //data.pm_2_5_at = msg1.content.value >> 16;
        //data.pm_1_at = msg1.content.value & 0xFFFF;
        msg_receive(&msg2);
        data.pm_1 = msg2.content.value & 0xFFFF;
        //data.concentration_unit_at = msg2.content.value & 0xFFFF;  
    }
    for(unsigned msg_cnt = 0; msg_cnt < 10; msg_cnt++){
        msg_receive(&msg1);
        data.pm_10 =  msg1.content.value >> 16;
        moy_pm10+=data.pm_10;//Stock les valeurs de pm10
        data.pm_2_5 = msg1.content.value & 0xFFFF;
        moy_pm2_5+=data.pm_2_5;//Stock les valeurs de pm10
        //data.pm_2_5_at = msg1.content.value >> 16;
        //data.pm_1_at = msg1.content.value & 0xFFFF;
        msg_receive(&msg2);
        data.pm_1 = msg2.content.value & 0xFFFF;
        moy_pm1+=data.pm_1;//Stock les valeurs de pm10
        //data.concentration_unit_at = msg2.content.value & 0xFFFF;  
    }
    
    /* unregister callback */
    pms7003_register_callback(&dev_pms7003,NULL,NULL); 
    
    SLEEP_SOFT(&dev_pms7003); 
         
    moy_pm10=moy_pm10/10.00; //divisé par 10 car 10 mesures
    tot_pm10 += (moy_pm10/10.00); // divisé par 10 pour avoir un chiffre a virgule
    moy_pm2_5=moy_pm2_5/10.00; //divisé par 10 car 10 mesures
    tot_pm2_5 += (moy_pm2_5/10.00); // divisé par 10 pour avoir un chiffre a virgule
    moy_pm1=moy_pm1/10.00; //divisé par 10 car 10 mesures
    tot_pm1 += (moy_pm1/10.00); // divisé par 10 pour avoir un chiffre a virgule
    
    _print_measurement_pms7003(moy_pm10,moy_pm2_5,moy_pm1);
}
#endif

#if CHOIX_CAPTEUR_HT == 1
static void Traitement_DHT22(void)
{
	/* Fonction récupèrant les valeur de temperature et d'humidité du capteur DHT22
	* et les retourne dans les variables globales tot_temp et tot_hum  
	*/
	
	int16_t temp, hum;
    char temp_s[10];
    char hum_s[10];
    
	if (dht_read(&dev_dht, &temp, &hum) != DHT_OK) //récupère les données
    {
    	puts("DHT22 : Error reading values");
    }
    
    tot_temp+=((float)temp/10.00);// Divisé par 10 pour avoir un chiffre à virgule
    tot_hum+=((float)hum/10.00);
    
    /***************** Affichage (optionnel) ******************/
    size_t n = fmt_s16_dfp(temp_s, temp, -1);
    temp_s[n] = '\0';
    n = fmt_s16_dfp(hum_s, hum, -1);
    hum_s[n] = '\0';

    printf("\n==> temp: %s°C | humidity: %s%%\n",temp_s, hum_s);
    /**********************************************************/
}
#elif CHOIX_CAPTEUR_HT == 2
static void Traitement_BME280(void)
{
	/* Fonction récupèrant les valeur de temperature et d'humidité du capteur BME280
	* et les retourne dans les variables globales tot_temp et tot_hum  
	*/


	/* read temperature, pressure [and humidity] values */
    float temperature = bmx280_read_temperature(&dev_bme);
    uint32_t pressure = bmx280_read_pressure(&dev_bme);
    #if defined(MODULE_BME280_SPI) || defined(MODULE_BME280_I2C)
    float humidity = bme280_read_humidity(&dev_bme);
    tot_hum+=(humidity/100.00);
    #endif
	tot_temp+=(temperature/100.00);// Divisé par 10 pour avoir un chiffre à virgule
	
	/***************** Affichage (optionnel) ******************/
    char str_temp[8];
    size_t len = fmt_s16_dfp(str_temp, temperature, -2);
    str_temp[len] = '\0';
	#if defined(MODULE_BME280_SPI) || defined(MODULE_BME280_I2C)
    char str_hum[8];
    len = fmt_s16_dfp(str_hum, humidity, -2);
    str_hum[len] = '\0';
    #endif
    
    printf("\n==> temp: %s°C | humidity: %s%%\n",str_temp,str_hum);
    printf("==> Pressure [Pa]: %ld PRIu32\n", pressure);
    /**********************************************************/
}
#endif

static void rtc_cb(void *arg)
{
    /* Fonction d'interruption de la fonction rtc_set_alarm : ajoute dans les 3 variables 
    * cnt_PERIOD le temps écoulé en seconde et compare ce temps à la période où l'action est
    * censé s'effectuer. Si la valeur est supérieur ou égale, effectue un msg_send pour activer
    * le thread. Sinon il retourne dormir un cycle = PERIOD
    */
    
    (void) arg;
    msg_t msg;
    
    cnt_Lora_PERIOD+=PERIOD;
    cnt_particule_PERIOD+=PERIOD;
    cnt_ht_PERIOD+=PERIOD;
   
   	if(cnt_Lora_PERIOD >= Lora_PERIOD)
   	{
   		cnt_particule_PERIOD=0;
    	cnt_ht_PERIOD=0;
    	cnt_Lora_PERIOD=0;
    	msg_send(&msg, sender_pid);
   	}
    else if((cnt_particule_PERIOD >= particule_PERIOD)&&(cnt_ht_PERIOD >= ht_PERIOD))
    {
    	cnt_particule_PERIOD=0;
    	cnt_ht_PERIOD=0;
    	#if CHOIX_CAPTEUR_PARTICULE == 0
        //Pas de capteur de particule
        #else
        msg_send(&msg, particule_pid); //Active le thread correspondant au pid
        #endif
        
        #if CHOIX_CAPTEUR_HT == 0
    	//Pas de capteur de temperature/humidite
    	#else
    	msg_send(&msg, ht_pid); //Active le thread correspondant au pid
    	#endif
    }
    else if(cnt_particule_PERIOD >= particule_PERIOD)
    {
    	cnt_particule_PERIOD=0;
    	#if CHOIX_CAPTEUR_PARTICULE == 0
        //Pas de capteur de particule
        #else
        msg_send(&msg, particule_pid); //Active le thread correspondant au pid
        #endif
    }
    else if(cnt_ht_PERIOD >= ht_PERIOD)
    {
    	cnt_ht_PERIOD=0;
    	#if CHOIX_CAPTEUR_HT == 0
    	//Pas de capteur de temperature/humidite
    	#else
    	msg_send(&msg, ht_pid); //Active le thread correspondant au pid
    	#endif
    }
    else
    {
    	_prepare_next_alarm();
    }
    //printf("PERIOD = %d\ncnt_Lora_PERIOD = %d\ncnt_particule_PERIOD = %d\ncnt_ht_PERIOD = %d\n",PERIOD,cnt_Lora_PERIOD,cnt_particule_PERIOD,cnt_ht_PERIOD);
}

static void _prepare_next_alarm(void)
{    
    /* Fonction initialisant le RTC timer. Verifie que la période Lora est plus grande que
    * la période de mesure du capteur ht et du capteur particule. Fixe la variable PERIOD en
    * prenant la plus petite période entre le capteur ht et le capteur particule ou en prenant
    * la valeur absolue de la différence entre la période du capteur ht et celle du capteur
    * particule si elle est plus petite que la plus petite période. A la fin du timer, va dans
    * la fonction rtc_cb
    */
    
    struct tm time;
    rtc_get_time(&time);
    /* set initial alarm */
    if((Lora_PERIOD < particule_PERIOD)||(Lora_PERIOD < ht_PERIOD))
    {
    	puts("[ERROR] : Lora_PERIOD < sensor_PERIOD\n");
    	return;
    }
    else if(particule_PERIOD < ht_PERIOD)
    {
    	PERIOD=particule_PERIOD;
    }
    else
    {
    	PERIOD=ht_PERIOD;
    }
    #if CHOIX_CAPTEUR_PARTICULE == 0
    	PERIOD=ht_PERIOD;
    #elif CHOIX_CAPTEUR_HT == 0
    	PERIOD=particule_PERIOD;
    #elif ((CHOIX_CAPTEUR_PARTICULE == 0)&&(CHOIX_CAPTEUR_HT == 0))
    	puts("[ERROR] : Aucun capteur\n");
    	return;
    #endif
    time.tm_sec += PERIOD;
    mktime(&time);
    rtc_set_alarm(&time, rtc_cb, NULL);
}

static void _send_message(void)
{
    /* Envoi un message via le protocole LoRaWAN 
    */
    
    /* Try to send the message */
    uint8_t ret = semtech_loramac_send(&loramac,lpp.buffer,lpp.cursor);
    
    if (ret != SEMTECH_LORAMAC_TX_DONE)  {
        printf("Cannot send message, ret code: %d\n", ret);
        return;
    }
}

static void *sender(void *arg)
{
	/* Thread gérant l'envoi via le protocole LoRaWAN
	* Stock dans l'objet cayenne la moyenne de toutes les valeurs mesurés par le capteur.
	* Reset tous les variables compteur(cnt) et les variables stockant les valeurs mesurés des 
	* 2 capteurs et envoi la trame via LoRaWAN.
	*/

    (void)arg;

    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);    

    while (1) 
    {       
        /* Wait the alarm */
        msg_receive(&msg); //Attend l'execution de msg_send(&msg, sender_pid)
        
        //Reset le lpp.buffer 
        cayenne_lpp_reset(&lpp);
        
        #if CHOIX_CAPTEUR_PARTICULE == 0
        //Pas de capteur de particule
        #else
        //add dans lpp.buffer les valeurs retourné par le capteur 
     	cayenne_lpp_add_temperature(&lpp,1,tot_pm2_5/cnt_particule); // Pour pm2.5
    	cayenne_lpp_add_temperature(&lpp,2,tot_pm10/cnt_particule); // Pour pm10   
   	 	#endif
   	 	
   	 	#if CHOIX_CAPTEUR_HT == 0
   	 	//Pas de capteur de temperature/humidite
   	 	#else
     	cayenne_lpp_add_temperature(&lpp,3,tot_temp/cnt_ht); //Affichage Temperature
    	cayenne_lpp_add_relative_humidity(&lpp,4,tot_hum/cnt_ht);// Humidite
    	#endif
    	
    	#if CHOIX_CAPTEUR_PARTICULE == 2
    	cayenne_lpp_add_temperature(&lpp,5,tot_pm1/cnt_particule); // Pour pm1
   	 	#endif

		/*************************** Important *****************************/
		//	Utilisation de la fonction "cayenne_lpp_add_temperature();"
		//  car le format est de 2 bytes et 0.1 donc possibilité d'aller
		//  jusqu'à la valeur 3276.8 car signé et notre valeur max est 1999.9
		/*******************************************************************/


		/* Reset */
		cnt_ht=0.00;
		cnt_particule=0.00;
		tot_pm2_5=0.00;
		tot_pm10=0.00;
		tot_temp=0.00;
		tot_hum=0.00;
		#if CHOIX_CAPTEUR_PARTICULE == 2
		tot_pm1=0.00;
		#endif

        /* Trigger the message send */
        puts("[SEND]");
        _send_message();
        
        #if CHOIX_CAPTEUR_PARTICULE == 0
        //Pas de capteur de particule
        #else
        msg_send(&msg, particule_pid); //Active le thread correspondant au pid
        #endif
        
        #if CHOIX_CAPTEUR_HT == 0
    	//Pas de capteur de temperature/humidite
    	#else
    	msg_send(&msg, ht_pid); //Active le thread correspondant au pid
    	#endif
		
		/* Schedule the next wake-up alarm */
        _prepare_next_alarm();
    }

    /* this should never be reached */
    return NULL;
}

static void *_recv(void *arg)
{
    /* Thread gérant les downlinks provenant de TTN :
    * Recoit le message en hexadécimal et change la période correspondante.
    */
    
    msg_init_queue(_recv_queue, RECV_MSG_QUEUE);
    (void)arg;
    while (1) 
    {
        /* blocks until some data is received */
        if(semtech_loramac_recv(&loramac)==SEMTECH_LORAMAC_RX_DATA)
        {
    	    loramac.rx_data.payload[loramac.rx_data.payload_len] = 0;
    	    printf("Data received: %s, port: %d\n",
               (char *)loramac.rx_data.payload, loramac.rx_data.port);
            switch(loramac.rx_data.payload[0])
            {
            	//Downlink :
            	case 'A'://41
            		Lora_PERIOD=60;//1 min
            	break;
            	
            	case 'B'://42
            		Lora_PERIOD=900;//15 min
            	break;
            	
            	case 'C'://43
            		Lora_PERIOD=1200;//20 min
            	break;
            	
            	case 'D'://44
            		Lora_PERIOD=1800;//30 min
            	break;
            	
            	
            	case 'Q'://51
            		particule_PERIOD=60;
            	break;
            	
            	case 'R'://52
            		particule_PERIOD=120;
            	break;
            	
            	case 'S'://53
            		particule_PERIOD=300;
            	break;
            	
            	case 'T'://54
            		particule_PERIOD=900;
            	break;
            	
            	case 'U'://55
            		particule_PERIOD=1200;
            	break;
            	
            	case 'a'://61
            		ht_PERIOD=60;
            	break;
            	
            	case 'b'://62
            		ht_PERIOD=120;
            	break;
            	
            	case 'c'://63
            		ht_PERIOD=300;
            	break;
            	
            	case 'd'://64
            		particule_PERIOD=900;
            	break;
            	
            	case 'e'://65
            		particule_PERIOD=1200;
            	break;
            	
            	default:
            		Lora_PERIOD=900;//par defaut 15 min
            		particule_PERIOD=600;//par defaut 2 min
            		ht_PERIOD=900;//par defaut 1 min
            	break;
            }
        }
    }
    return NULL;
}

#if CHOIX_CAPTEUR_PARTICULE == 0
//Pas de capteur de particule
#else
static void *_particule(void *arg)
{
	/* Thread gérant le capteur particule (SDS011 ou PMS7003)
	* Appelle la fonction traitement du capteur choisi. Compte le nombre de fois
	* que la fonction Traitement_CAPTEUR est effectué. Fais la moyenne tout les 10
	* cycles pour éviter le débordement de la variable tot_pm10 et tot_pm2_5
	*/
	
	(void)arg;
    msg_t msg;  
    msg_t msg_queue[PARTICULE_MSG_QUEUE];
    msg_init_queue(msg_queue, PARTICULE_MSG_QUEUE); 
    
    while (1) {
        msg_receive(&msg); //Attend l'execution de msg_send(&msg, particule_pid)
    #if CHOIX_CAPTEUR_PARTICULE == 1
     	Traitement_SDS011();
    #elif CHOIX_CAPTEUR_PARTICULE == 2
     	Traitement_PMS7003();
    #else
     	puts("[ERROR] : choix capteur function ==> _particule");
     	return;
    #endif

		cnt_particule++;//Compte le nombre de mesure pour réaliser la moyenne
		if(cnt_particule >= 10.00) 
		{
			/* Au bout de 10 cycle de mesure, on fait la moyenne car 
			risque d'overflow du type float */
			tot_pm2_5=tot_pm2_5/cnt_particule;
			tot_pm10=tot_pm10/cnt_particule;
			#if CHOIX_CAPTEUR_PARTICULE == 2
			tot_pm1=tot_pm1/cnt_particule;
			#endif
			cnt_particule=1.00;
		}
        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();
    }
    /* this should never be reached */
    return NULL;
}
#endif

#if CHOIX_CAPTEUR_HT == 0
//Pas de capteur de temperature/humidite
#else
static void *_ht(void *arg)
{
	/* Thread gérant le capteur humidite/temperature (DHT22 ou BME280)
	* Appelle la fonction traitement du capteur choisi. Compte le nombre de fois
	* que la fonction Traitement_CAPTEUR est effectué. Fais la moyenne tout les 10
	* cycles pour éviter le débordement de la variable tot_temp et tot_hum
	*/
	
	(void)arg;
    msg_t msg; 
    msg_t msg_queue[HT_MSG_QUEUE];
    msg_init_queue(msg_queue, HT_MSG_QUEUE);

    while (1) {
        msg_receive(&msg); //Attend l'execution de msg_send(&msg, ht_pid)
    #if CHOIX_CAPTEUR_HT == 1
     	Traitement_DHT22();
	#elif CHOIX_CAPTEUR_HT == 2
		Traitement_BME280();
	#else
		puts("[ERROR] : choix capteur function ==> _ht");
     	return;
	#endif
		cnt_ht++;//Compte le nombre de mesure pour réaliser la moyenne 
		if(cnt_ht >= 10.00) 
		{
			/* Au bout de 10 cycle de mesure, on fait la moyenne car 
			risque d'overflow du type float */
			tot_temp=tot_temp/cnt_ht;
			tot_hum=tot_hum/cnt_ht;
			cnt_ht=1.00;
		}
		
        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();
    }
    /* this should never be reached */
    return NULL;
}
#endif
