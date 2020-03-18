/* This program is a simple test of the UART protocol for
the pms7003 sensor, it show all the datas frame receive on the RX board.
This program is an easy way to understand how the UART communication works.
For the synchronization between the RX callback and the main fonction, we use mutex
but a messaging system can be used with the static PID of the main Thread */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "thread.h"
#include "mutex.h"
#include "periph/uart.h"

#define PMS_UART  UART_DEV(1)

#define BAUDRATE (9600)


typedef struct {
    uint8_t buf_mem[32];      
    int idx; 
    uint16_t checksum;
    bool available;
    mutex_t key_mutex;
} frame_type;

static frame_type frame_main;


static void rx_cb(void *arg, uint8_t c)
{
   
   frame_type *frame = (frame_type *)arg; 
    if(((c != 0x42 ) && (frame->idx==0)) || (frame->available) ){
        return;
    }
    if(((c != 0x4d ) && (frame->idx==1)) || (frame->available) ){
        return;
    }

    if(((c != 28 ) && (frame->idx==3)) || (frame->available) ){
        return;
    }

    
    frame->buf_mem[frame->idx]=c; 
    if (frame->idx < 30){
        frame->checksum += c ; 
    }
    

    if(frame->idx == 31){

    	frame->available = true ; 
    	mutex_unlock(&frame->key_mutex);
    }

    frame->idx = frame->idx + 1; 

}



int main(void)

{   

    printf("Connection between UART PMS7003 \n");
    printf("====================================\n");
	
    frame_main.idx = 0 ; 
    frame_main.available = false ; 
    frame_main.checksum = 0 ; 
    mutex_init(&frame_main.key_mutex);

    /* UART2 : PMS terminal */
    if ( uart_init(PMS_UART, BAUDRATE, rx_cb,&frame_main) != 0){
        printf("[INIT] Error\n");
    }else {
        printf("[INIT] Ok\n");
    }


    
    while(1) {
            mutex_lock(&frame_main.key_mutex);
            printf("\n-------------------\n");
            for (int i = 0; i < 32; ++i)
            {
                printf("-> Data[%d] = %d \n",i,frame_main.buf_mem[i]);
            }
            uint16_t check = frame_main.buf_mem[30] << 8 | frame_main.buf_mem[31] ; 
            printf("\nchecksum sensor = %d\n",check);
            printf("My checksum = %d\n",frame_main.checksum);
            if (check == frame_main.checksum){
                printf("[Checksum] Ok\n"); 
            } 
            frame_main.idx = 0 ;
            frame_main.available = false ;
            frame_main.checksum = 0 ; 
        }

    return 0;
}
