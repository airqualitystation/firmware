
#ifndef PMS7003_INTERNAL_H
#define PMS7003_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

//Uart baudrate
#define PMS7003_UART_BAUDRATE             (9600)

//Frame length receive and frame length response command
#define FRAME_LEN               (32)
#define FRAME_CMD_LEN           (8)
#define FRAME_HOST_LEN          (7)
#define SENSOR_LEN              (28)
#define SENSOR_CMD_LEN          (4)
//Commmandes
#define _POWER                  (0xE4)
#define _MODE                   (0xE1)
#define _READ_PASSIVE           (0xE2)
//Commandes options
#define _PASSIVE                (0x00)
#define _ACTIVE                 (0x01)
#define _SLEEP                  (0x00)
#define _WAKEUP                 (0x01)
//Start frame characters
#define HEAD1                  (0x42)
#define HEAD2                  (0x4D)                  

//FRAME INDEX
#define START_HEAD1_IDX            (0)
#define START_HEAD2_IDX            (1)
#define LENGTH_HIGH_IDX            (2)
#define LENGTH_LOW_IDX             (3) 
//concentration standard particules 
#define STANDARD_PM1_HIGH_IDX      (4)
#define CMD_MODE_IDX               (4)
#define STANDARD_PM1_LOW_IDX       (5)
#define STANDARD_PM25_HIGH_IDX     (6)
#define STANDARD_PM25_LOW_IDX      (7)

#define CHEKSUM_CMD_HIGH_IDX       (6)
#define CHEKSUM_CMD_LOW_IDX        (7)

#define STANDARD_PM10_HIGH_IDX     (8)
#define STANDARD_PM10_LOW_IDX      (9)
//Concentration under atmospheric evironment 
#define ATMOS_PM1_HIGH_IDX        (10)
#define ATMOS_PM1_LOW_IDX         (11)
#define ATMOS_PM25_HIGH_IDX       (12)
#define ATMOS_PM25_LOW_IDX        (13)  
#define ATMOS_PM10_HIGH_IDX       (14)
#define ATMOS_PM10_LOW_IDX        (15)
//particules number with diameter
#define DIA_03_HIGH_IDX           (16)
#define DIA_03_LOW_IDX            (17)
#define DIA_05_HIGH_IDX           (18)
#define DIA_05_LOW_IDX            (19)  
#define DIA_10_HIGH_IDX           (20)
#define DIA_10_LOW_IDX            (21)
#define DIA_25_HIGH_IDX           (22)
#define DIA_25_LOW_IDX            (23)
#define DIA_50_HIGH_IDX           (24)
#define DIA_50_LOW_IDX            (25)  
#define DIA_100_HIGH_IDX          (26)
#define DIA_100_LOW_IDX           (27)
//Unique device ID
#define ID_HIGH_IDX               (28)
#define ID_LOW_IDX                (29)
//Checksum
#define CHECKSUM_HIGH_IDX         (30)
#define CHECKSUM_LOW_IDX          (31)


//ERRORS :


enum {
  PMS7003_OK = 1,
  PMS7003_ERROR = -6,
  PMS7003_UART_ON_ERROR = -1,
  PMS7003_UART_OFF_ERROR = -2,
  PMS7003_CMD_INVALIDE = -3,
  PMS7003_CHECKSUM_ERROR = -4,
  PMS7003_GPIO_ERROR = -5,
};


#define PMS7003_DEVID_WILDCARD            (0xFFFF)


#ifdef __cplusplus
}
#endif

#endif 
