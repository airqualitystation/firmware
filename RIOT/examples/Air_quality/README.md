# Set up the Air quality project
## choix_config.h
In the file called "choix_config.h", you can choose:
* which sensor or sensors you want to use
* the sensor measurement period
* the frequency of transmission of datas with the lorawan protocol

## Makefile
Add the RIOT libraries that are useful for the Air quality project.
You need to edit:
* BOARD (default: b-l072z-lrwan1)
* DRIVER (default: sx1276)
* LORA_REGION (default: EU868)
* DEVEUI, APPEUI and APPKEY

## main.c
the program allows to measure the air quality PM2.5, PM10 (and PM1 only with the pms7003), the temperature, the humidity and sends the datas with the lorawan protocol.

