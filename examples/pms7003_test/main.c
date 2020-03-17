
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "xtimer.h"

#include "pms7003.h"
#include "pms7003_params.h"
#include "msg.h"
#include "thread.h"


static pms7003_t dev; 


static void _print_measurement_standard(pms7003_data_t *data)
{

    uint16_t pm10_ug_int = data->pm_10 / 10;
    uint16_t pm10_ug_dec = data->pm_10 - 10 * pm10_ug_int;
    uint16_t pm2_5_ug_int = data->pm_2_5 / 10;
    uint16_t pm2_5_ug_dec = data->pm_2_5 - 10 * pm2_5_ug_int;

    uint16_t pm1_ug_int = data->pm_1 / 10;
    uint16_t pm1_ug_dec = data->pm_1 - 10 * pm1_ug_int;

    printf("==>  PM1: %d.%0d ug/m^3 | ==>  PM2.5: %d.%0d ug/m^3 | PM10: %d.%0d ug/m^3\n ",pm1_ug_int, pm1_ug_dec,pm2_5_ug_int, pm2_5_ug_dec, pm10_ug_int, pm10_ug_dec);
}





void measure_cb_pm_standard(pms7003_data_t *data, void *ctx)
{
    msg_t msg1 = { .content.value = (((uint32_t)data->pm_10) << 16 | data->pm_2_5) };
    msg_t msg2 = { .content.value = (((uint32_t)data->pm_1 )& 0xFFFF)};

    kernel_pid_t target_pid = (int)ctx;
    msg_send(&msg1, target_pid);
    msg_send(&msg2, target_pid);
}




int main(void)
{
    
    
    puts("\nPMS7003 test application\n");


    /* initialize the driver */
    if(pms7003_init(&dev, &pms7003_params[0])>=0){printf("[INIT] Ok\n");}else{printf("[INIT] Error\n");}
     

while(1){ 

  // In order to be sure that the device is not sleep 
  // Soft because we use the uart interface to send cmd at the device
  // You can use the hardware by connecting the coresponding pin to your board <WAKEUP_HARD>
    WAKEUP_SOFT(&dev); 

  // The active mode is better accurate than passive
  // The active mode need a callback register procedure.
  // -> If you want use the passive mode, then you need to creat your own RX_CALLBACK and attached to the device
  // For more details of the implementation of rx_callback for the passive mode, see <pms7003.c> in the driver folder.
  // In the passive mode you need to send read_passive command to obtain just one measure...
    ACTIVE_MODE(&dev); 
    

// Register the corresponding callback <measure_cb_pm_standard> for the measurement.
// If you want unregister the callback use this :
    // ->  pms7003_register_callback(&dev,NULL,NULL);

    pms7003_register_callback(&dev,measure_cb_pm_standard,(void*)(int)thread_getpid());



    printf("Callback mode \n");
    
    msg_t msg1, msg2;
    for(int i=0;i<10;i++){
        msg_receive(&msg1);
        pms7003_data_t data;
        data.pm_10 =  msg1.content.value >> 16;
        data.pm_2_5 = msg1.content.value & 0xFFFF;
        msg_receive(&msg2);
        data.pm_1 = msg2.content.value & 0xFFFF;

        printf("msg from callback: ");
        _print_measurement_standard(&data);
    }

    SLEEP_SOFT(&dev);
    wait_second(15); 


  }
  
    return 0;
}
