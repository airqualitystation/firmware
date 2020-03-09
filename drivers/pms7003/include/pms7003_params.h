
#ifndef PMS7003_PARAMS_H
#define PMS7003_PARAMS_H

#include "board.h"
#include "periph/uart.h"
#include "saul_reg.h"
#include "pms7003.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   Set default configuration parameters for the SDS011 driver
 * @{
 */
#ifndef PMS7003_PARAM_UART_DEV
#define PMS7003_PARAM_UART_DEV       (UART_DEV(1))
#endif
#ifndef PMS7003_PARAM_SET_PIN
#define PMS7003_PARAM_SET_PIN        (GPIO_PIN(PORT_A, 8))  // D7
#endif
#ifndef PMS7003_PARAM_PWR_LEVEL
#define PMS7003_PARAM_PWR_LEVEL     (true)
#endif

#ifndef PMS7003_PARAMS
#define PMS7003_PARAMS               { .uart    = PMS7003_PARAM_UART_DEV, \
                                      .set_pin = PMS7003_PARAM_SET_PIN, \
                                      .pwr_level  = PMS7003_PARAM_PWR_LEVEL, \
                                     }
#endif

#ifndef PMS7003_SAUL_INFO
#define PMS7003_SAUL_INFO            { .name = "PMS7003" }
#endif








/**
 * @brief   Allocate and configure entries to the SAUL registry
 */
/*saul_reg_info_t pms7003_saul_info[] =
{
    PMS7003_SAUL_INFO
};
*/
#ifdef __cplusplus
}
#endif

#endif /* PMS7003_PARAMS_H */
/** @} */

