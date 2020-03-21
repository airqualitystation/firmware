
#include <stdio.h>

#include "xtimer.h"
#include "timex.h"
#include "ads101x.h"
#include "ads101x_params.h"
#include "ads101x_regs.h"

#define SLEEP       (100 * US_PER_MS)

static ads101x_t dev;
static ads101x_alert_t alert_dev;

static void alert_cb(void *arg)
{
    (void)arg;
    puts("\n[Alert!]");
}

int main(void)
{
/*******************************************/
/**************VARIABLES *******************/
    int16_t data = 0;
    float Umax, pourcentage_of_Umax, autonomy, U1,U2, I ;
    int Mult_FLOATtoINT,e_part,d_part;
    Umax = pourcentage_of_Umax = autonomy = U1 = U2 = I = 0 ;
    Mult_FLOATtoINT = e_part = d_part = 0; 
/*******************************************/
/*******************************************/
    puts("ADS101X analog to digital driver test application\n");
    printf("Initializing ADS101x analog to digital at I2C_DEV(%i)... ",
           ads101x_params->i2c);

    if (ads101x_init(&dev, ads101x_params) == ADS101X_OK) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return -1;
    }

    printf("Initializing ADS101x alert at I2C_DEV(%i)... ",
           ads101x_alert_params->i2c);

    if (ads101x_alert_init(&alert_dev, ads101x_alert_params) == ADS101X_OK) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return -1;
    }

    printf("Enabling alert interrupt: ");
    if (ads101x_enable_alert(&alert_dev, alert_cb, NULL) == ADS101X_OK) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return -1;
    }

    while (1) {
        printf("[Raw CH0] ->   ");
        ads101x_set_mux_gain(&dev, ADS101X_AIN0_SINGM | ADS101X_PGA_FSR_4V096);
        if (ads101x_read_raw(&dev, &data) == ADS101X_OK) {
        	Umax = 4.096 ; 
	    	pourcentage_of_Umax = (float)data / (32767.0) ;
	  		/***************************************/
	  		/******* Values to send by LoRa ********/
	    	U1 = Umax * pourcentage_of_Umax ;
        	autonomy = U1 - 3.2 ; 
	    	if (autonomy <= 0 ){
				autonomy = 0;
	    	}else{
			autonomy = ((1-0.896)+ autonomy); 
	    	}
	    	autonomy = autonomy * 100 ; 
	    	
	   		/********************************/
	    	/*****  Serial port display ****/

	    	Mult_FLOATtoINT = U1 * 1000 ;				// 10^-3 precision
	    	e_part = Mult_FLOATtoINT / 1000 ;			// value of Volt
	    	d_part = Mult_FLOATtoINT - (e_part*1000);	// value of mVolt
	    	printf("U1 : %i.%i V \tAutonomy : %i pourcent",e_part,d_part,(int)autonomy);
	    	printf("\t"); 
	    	/********************************/
	    	/********************************/
        }
        else {
            puts("[Fail]");
            continue;
        }


        printf("[Raw CH1] ->   ");
        ads101x_set_mux_gain(&dev, ADS101X_AIN1_SINGM | ADS101X_PGA_FSR_4V096);
        if (ads101x_read_raw(&dev, &data) == ADS101X_OK) {
        	Umax = 4.096 ; 
	    	pourcentage_of_Umax = (float)data / (32767.0) ;
	    	U2 = Umax * pourcentage_of_Umax ;
        
        	/********************************/
	    	/**** Serial port display ******/

	    	Mult_FLOATtoINT = U2 * 1000 ; 				// 10^-3 precision
	    	e_part = Mult_FLOATtoINT / 1000 ; 			// value of Volt
	    	d_part = Mult_FLOATtoINT - (e_part*1000);	// value of mVolt
	    	printf("U2 : %i.%i V \t",e_part,d_part); 
        }
        else {
            puts("[Fail]");
            continue;
        }

/*****************************************************/
/******* Differential measurement (CH0 - CH1) ********/
        printf("[Raw diff] ->   ");
        ads101x_set_mux_gain(&dev, ADS101X_AIN0_DIFFM_AIN1 | ADS101X_PGA_FSR_0V256);
        if (ads101x_read_raw(&dev, &data) == ADS101X_OK) {
        	Umax = 0.256 ; 
	    	pourcentage_of_Umax = (float)data / (32767.0) ;

	    	/********************************/
	  		/**** Values to send by LoRa ****/
	    	I = Umax * pourcentage_of_Umax ;
        

        	/********************************/
	    	/**** Serial port display ******/

	    	Mult_FLOATtoINT = I * 100000 ; 				// 10^-5 precision
	    	e_part = Mult_FLOATtoINT / 100 ; 			// value of mA 
	    	d_part = Mult_FLOATtoINT - (e_part*100);	// value of uA
	    	printf("I : %i.%i mA \n\n",e_part,d_part); 
        }
        else {
            puts("[Fail]");
            continue;
        }


/*
        printf("[Raw diff] ->   ");
	    I = U1 - U2;
	    Mult_FLOATtoINT = I * 100000 ; 				// 10^-5 precision
	    e_part = Mult_FLOATtoINT / 100 ; 			// value of mA 
	    d_part = Mult_FLOATtoINT - (e_part*100);	// value of uA
	    printf("I : %i.%i mA \n\n",e_part,d_part); 
*/



        xtimer_usleep(SLEEP);
    }

    return 0;
}
