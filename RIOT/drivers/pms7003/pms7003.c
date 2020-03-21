#include <string.h>

#include "assert.h"
#include "pms7003.h"
#include "periph/uart.h"


#define TIMEOUT (3U * US_PER_SEC)

uint8_t cmd_sleep[FRAME_HOST_LEN] = {HEAD1,HEAD2,_POWER, 0x00 ,_SLEEP, 0x01 , 0x73 };
uint8_t cmd_wakeup[FRAME_HOST_LEN] = {HEAD1,HEAD2,_POWER, 0x00 ,_WAKEUP, 0x01 , 0x74 };
uint8_t cmd_active[FRAME_HOST_LEN] = {HEAD1,HEAD2,_MODE, 0x00 ,_ACTIVE, 0x01 , 0x71 };
uint8_t cmd_passive[FRAME_HOST_LEN] = {HEAD1,HEAD2,_MODE, 0x00 ,_PASSIVE, 0x01 , 0x70 };
uint8_t cmd_read[FRAME_HOST_LEN] = {HEAD1,HEAD2,_READ_PASSIVE, 0x00 ,0x00, 0x01 , 0x71 };


/* Uart receive callback */
static void rx_cb(void *arg, uint8_t c)
{
   
   pms7003_t *dev = (pms7003_t *)arg; 

   if(dev->reset){
    dev->idx=0;
    dev->checksum = 0;
    dev->reset = false;
   }

   if((c != HEAD1 ) && (dev->idx==0)){ 
        return;
    }
   if((c != HEAD2 ) && (dev->idx==1)){
        return;
    }
    
    //PAYLOAD//
    dev->buf_mem[dev->idx]=c; 
    if (dev->idx < FRAME_LEN-2){
        dev->checksum += c ; 
    }
    
  	
    if(dev->idx == FRAME_LEN-1){
      uint16_t sensor = dev->buf_mem[CHECKSUM_HIGH_IDX ] << 8 | dev->buf_mem[CHECKSUM_LOW_IDX ] ; 
      if ((dev->checksum == sensor)&&(dev->buf_mem[CHECKSUM_HIGH_IDX ]!=0)&&(dev->buf_mem[CHECKSUM_LOW_IDX ]!=0)){
            //Data
      		pms7003_data_t measure;

      		measure.pm_1 =  (dev->buf_mem[STANDARD_PM1_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[STANDARD_PM1_LOW_IDX] ;
            measure.pm_2_5 = (dev->buf_mem[STANDARD_PM25_HIGH_IDX] << 8) |
                                     dev->buf_mem[STANDARD_PM25_LOW_IDX] ;
            measure.pm_10  = (dev->buf_mem[STANDARD_PM10_HIGH_IDX] << 8) |
                                     dev->buf_mem[STANDARD_PM10_LOW_IDX] ;

            measure.pm_1_at =  (dev->buf_mem[ATMOS_PM1_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[ATMOS_PM1_LOW_IDX] ;
            measure.pm_2_5_at =  (dev->buf_mem[ATMOS_PM25_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[ATMOS_PM25_LOW_IDX] ;

            measure.nb_0_3 =  (dev->buf_mem[DIA_03_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[DIA_03_LOW_IDX] ;
            measure.nb_0_5 =  (dev->buf_mem[DIA_05_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[DIA_05_LOW_IDX] ;
            measure.nb_1 =  (dev->buf_mem[DIA_10_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[DIA_10_LOW_IDX] ;
            measure.nb_2_5 =  (dev->buf_mem[DIA_25_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[DIA_25_LOW_IDX] ;
            measure.nb_0_5 =  (dev->buf_mem[DIA_50_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[DIA_50_LOW_IDX] ;
            measure.nb_10 =  (dev->buf_mem[DIA_100_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[DIA_100_LOW_IDX] ;

            measure.concentration_unit_at =  (dev->buf_mem[ATMOS_PM10_HIGH_IDX] << 8 ) |
                                     dev->buf_mem[ATMOS_PM10_LOW_IDX] ;
            
            dev->cb(&measure,dev->cbctx);
      	    dev->reset = true ; 
            
       }
       else{
       		dev->reset=true;
       		return;
       } 
       
    }

    dev->idx = dev->idx + 1; 
}


static void rx_cb_cmd(void *arg, uint8_t c)
{
   
   pms7003_t *dev = (pms7003_t *)arg; 

   	if((c != HEAD1 ) && (dev->idx==0)) {
        return;
    }
    if((c != HEAD2 ) && (dev->idx==1)){
       	return;
    }

    dev->buf_mem[dev->idx]=c; 
    if (dev->idx < FRAME_CMD_LEN-2){
       	dev->checksum += c ; 
   	}


   	if((dev->idx==FRAME_CMD_LEN-1)&&(dev->buf_mem[LENGTH_LOW_IDX]==SENSOR_CMD_LEN)){
   		mutex_unlock(&dev->set);
   		return; 
   	}


   	if(dev->idx == FRAME_LEN-1){    		
   		mutex_unlock(&dev->set); 
   		return; 
    }

    dev->idx = dev->idx + 1; 
   
}








int send_cmd(pms7003_t *dev,working_mode_t cmd_type){

	/*UNREGISTER THE CALLBACK MEASURE ON THE UART*/
	/*Only if the register callback was init before in the main for the active mode*/
	if(dev->cb != NULL){
		 if(uart_init(dev->params.uart, PMS7003_UART_BAUDRATE,NULL,NULL) != 0) {
    		return  PMS7003_UART_OFF_ERROR ;
    	}
	}

	uint16_t my_checksum=0; uint8_t recv_frm[FRAME_CMD_LEN];
	uint16_t sensor_checksum;  
	uint8_t *ptr; //bool response_ok =false;

	switch(cmd_type){
    	case WAKEUP:
    		ptr=cmd_wakeup;
    		break;
    	case SLEEP:
    		ptr=cmd_sleep;
      		break;
    	case ACTIVE:
    		ptr=cmd_active;
      		break;
    	case PASSIVE:
    		ptr=cmd_passive;
     		break;
    	default:
      		return PMS7003_CMD_INVALIDE; 
  	}

 /***************************/
  	/*INITIALISATION*/ 
	dev->idx=0;
    dev->checksum = 0;
    dev->cmd = cmd_type;

/****************************/

    /*SET THE CALLBACK CMD ON THE UART */
    if(uart_init(dev->params.uart, PMS7003_UART_BAUDRATE, rx_cb_cmd, dev) != 0) {
    	return  PMS7003_UART_ON_ERROR ;
    }

    xtimer_ticks32_t last_time = xtimer_now();
    xtimer_ticks32_t time=last_time; 
    while(1){

    	if(time.ticks32 >= last_time.ticks32 + TIMEOUT){
    		if(uart_init(dev->params.uart, PMS7003_UART_BAUDRATE,NULL,NULL) != 0) {
    			return  PMS7003_UART_OFF_ERROR ;
    		}
    		return PMS7003_ERROR;
    	}
    	/*Write on the Uart the coresponding CMD*/
    	uart_write(dev->params.uart,ptr,FRAME_HOST_LEN);
    /************************/	
    	/*->Wait a frame of 32 bytes*/
    	mutex_lock(&dev->set);
 	/************************/

    	for(int i=0;i<FRAME_CMD_LEN-2;i++){
       		my_checksum = dev->buf_mem[i] + my_checksum;
  		}
  		//Checksum test
  		sensor_checksum = dev->buf_mem[CHEKSUM_CMD_HIGH_IDX] << 8 | dev->buf_mem[CHEKSUM_CMD_LOW_IDX] ;

  		if(my_checksum == sensor_checksum){

      		memcpy(recv_frm,dev->buf_mem,FRAME_CMD_LEN);
      		//CMD test
    		if((recv_frm[LENGTH_LOW_IDX ]  == SENSOR_CMD_LEN) && ((recv_frm[CMD_MODE_IDX ] == _POWER) ||
    		(recv_frm[CMD_MODE_IDX ] == _MODE) || (recv_frm[CMD_MODE_IDX ]== _READ_PASSIVE))){
    			if(uart_init(dev->params.uart, PMS7003_UART_BAUDRATE,NULL,NULL) != 0) {
    				return  PMS7003_UART_OFF_ERROR ;
    			}
    			return PMS7003_OK;
    		
    		}	
  		}
  		//Exception if WEKEUP cmd
    	if(cmd_type==WAKEUP){
    		if(uart_init(dev->params.uart, PMS7003_UART_BAUDRATE,NULL,NULL) != 0) {
    			return  PMS7003_UART_OFF_ERROR ;
    		}
    		return PMS7003_OK;
    	}

    	/***********************/
    	/*Next try*/
    	dev->checksum=0;
    	dev->idx=0;
    	my_checksum=0;
    	/***********************/
    	time = xtimer_now();
    }
 
}




int pms7003_init(pms7003_t *dev, const pms7003_params_t *params)
{
    assert((dev != NULL) && (params != NULL) && (params->uart < UART_NUMOF));

    if ((params->set_pin != GPIO_UNDEF) &&
        (gpio_init(params->set_pin, GPIO_OUT) != 0)) {
        return PMS7003_GPIO_ERROR;
    }

    memcpy(&dev->params, params, sizeof(pms7003_params_t));

    mutex_init(&dev->set);
    mutex_init(&dev->read_passive); 
    dev->cb = NULL;
    pms7003_set_on(dev);
    dev->reset = true;
    dev->cmd = INCCONU; 

    return PMS7003_OK;
}


int pms7003_register_callback(pms7003_t *dev, pms7003_callback_t cb, void *ctx)
{
    assert(dev != NULL);
    dev->cbctx = ctx;
    dev->cb = cb; //mesure cb 

    if (uart_init(dev->params.uart, PMS7003_UART_BAUDRATE,rx_cb,dev) != 0) {
      return PMS7003_UART_ON_ERROR;
    }
    return PMS7003_OK;
}

void pms7003_set_on(pms7003_t *dev){

    assert(dev != NULL);
    if(dev->params.set_pin != GPIO_UNDEF ){

        gpio_write(dev->params.set_pin,1);
    }
}

void pms7003_set_off(pms7003_t *dev){

    assert(dev != NULL);
    if(dev->params.set_pin != GPIO_UNDEF ){
        
        gpio_write(dev->params.set_pin,0);
    }

}


int WAKEUP_SOFT(pms7003_t *dev){
    
   int feedback = send_cmd(dev,WAKEUP);
   if(feedback>=0){
   	printf("[WAKEUP SOFT] Ok\n");
   }else{
   	printf("[WAKEUP SOFT] Error\n"); 
   }
   return feedback;
}


void WAKEUP_HARD(pms7003_t *dev){
    pms7003_set_on(dev);
    dev->cmd = WAKEUP ;  
  printf("[WAKEUP HARD] Ok\n");
}


int SLEEP_SOFT(pms7003_t *dev){
   int feedback = send_cmd(dev,SLEEP);
   if(feedback>=0){
   	printf("[SLEEP SOFT] Ok\n");
   }else{
   	printf("[SLEEP SOFT] Error\n");
   }
   return feedback;
}



void SLEEP_HARD(pms7003_t *dev){
    pms7003_set_on(dev);
    dev->cmd = SLEEP ;  
  	printf("[SLEEP HARD] Ok\n");
}


int ACTIVE_MODE(pms7003_t *dev){
   int feedback = send_cmd(dev,ACTIVE);
   if(feedback>=0){
   	printf("[ACTIVE SOFT] Ok\n");
   }else{
   	printf("[ACTIVE SOFT] Error\n");
   }
   return feedback;
}




void wait_second(int tps){
    int periode = tps*US_PER_SEC; 
    xtimer_ticks32_t time = xtimer_now();
    xtimer_ticks32_t real=time;
    while(real.ticks32 <= time.ticks32+periode){
      real = xtimer_now();
    }
}


void wait_ms(int tps){
    int periode = tps*US_PER_MS; 
    xtimer_ticks32_t time = xtimer_now();
    xtimer_ticks32_t real=time;
    while(real.ticks32 <= time.ticks32+periode){
      real = xtimer_now();
    }
}

