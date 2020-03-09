

#ifndef PMS7003_H
#define PMS7003_H

#include <stdbool.h>

#include "periph/gpio.h"
#include "periph/uart.h"
#include "mutex.h"
#include "pms7003_internal.h"
#include "pms7003_params.h"
#include "xtimer.h"
#include "timex.h"


#ifdef __cplusplus
extern "C" {
#endif





typedef enum commandes{
    WAKEUP =0,
    SLEEP = 1,
    ACTIVE = 2,
    PASSIVE = 3,
    READ_PASSIVE =0,
    INCCONU = -1,
}working_mode_t;


/** @} */

/**
 * @brief   PMS7003 configuration
 */
typedef struct {
    uart_t   uart;      
    gpio_t   set_pin; 
    bool     pwr_level;   
} pms7003_params_t;


static const  pms7003_params_t pms7003_params[] =
{
    PMS7003_PARAMS
};



typedef struct {
    uint16_t pm_1;    /**< Particulate Matter 1 concentration [0.1µg/m^3] */
    uint16_t pm_2_5;  /**< Particulate Matter 2.5 concentration [0.1µg/m^3] */
    uint16_t pm_10;   /**< Particulate Matter 10 concentration [0.1µg/m^3] */
    uint16_t pm_1_at;
    uint16_t pm_2_5_at;
    uint16_t concentration_unit_at;
    uint16_t nb_0_3;
    uint16_t nb_0_5;
    uint16_t nb_1;
    uint16_t nb_2_5;
    uint16_t nb_5;
    uint16_t nb_10; 
} pms7003_data_t;


typedef void (*pms7003_callback_t)(pms7003_data_t *data, void *ctx);





typedef struct {
    uint8_t buf_mem[32];      
    int idx; 
    uint16_t checksum;
    bool reset; 
    bool cmd_mode;
    bool available;
    uint16_t ID; 
    working_mode_t cmd;
    mutex_t set;
    mutex_t read_passive; 
    pms7003_callback_t cb;            
    void     *cbctx; 
    pms7003_params_t   params;       
} pms7003_t;






int pms7003_init(pms7003_t *dev, const pms7003_params_t *params);


int pms7003_register_callback(pms7003_t *dev, pms7003_callback_t cb, void *ctx);


void pms7003_set_on(pms7003_t *dev);

void pms7003_set_off(pms7003_t *dev);

int send_cmd(pms7003_t *dev,working_mode_t cmd_type);

void wait_second(int tps);
int PASSIVE_MODE(pms7003_t *dev);
int ACTIVE_MODE(pms7003_t *dev);
void SLEEP_HARD(pms7003_t *dev);
int SLEEP_SOFT(pms7003_t *dev);
void WAKEUP_HARD(pms7003_t *dev);
int WAKEUP_SOFT(pms7003_t *dev);
//int READ_IN_PASSIVE_MODE(pms7003_t *dev,pms7003_data_t *measure,int nb);

#ifdef __cplusplus
}
#endif

#endif /* PMS7003_H */
/** @} */

