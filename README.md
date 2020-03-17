# Air Quality Station :: Firmware
This project contains the RIOT firmware of the Air Quality Station

## Hardware requirements
* [STM32 Nucleo B-L072Z-LRWAN1](https://www.st.com/en/evaluation-tools/b-l072z-lrwan1.html)
* [SDS011](https://cdn-reichelt.de/documents/datenblatt/X200/SDS011-DATASHEET.pdf)
* [PMS7003](https://usermanual.wiki/Document/PMS7003seriesdata20manualEnglishV25.1220636559/view)
* [DHT22](https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf)
* [BME280](https://www.waveshare.com/w/upload/7/75/BME280_Environmental_Sensor_User_Manual_EN.pdf)
* [Solar Panel 180x80 mm](https://www.gotronic.fr/art-cellule-solaire-sol2w-18995.htm#complte_desc)
* [LiPo Battery 3.7 V / 2000 mAh](https://www.sparkfun.com/datasheets/Batteries/UnionBattery-2000mAh.pdf)
* [Solar Power Manager 5V](https://www.dfrobot.com/product-1712.html)

## Build and flash the firmware
* First, you need to clone, or download, [the RIOT library](https://github.com/RIOT-OS/RIOT.git)
* To build the project, you need to copy the files called "drivers" and "examples" in your RIOT file
* You need to merge and replace files

Now, you can compile the Air_quality project in your board with the command: 'make flash' or 'make flash term' if you have pyterm.

## Set up the Air quality project
### choix_config.h
In the file called "choix_config.h", you can choose:
* which sensor or sensors you want to use
* the sensor measurement period
* the frequency of transmission of datas with the lorawan protocol

### Makefile
Add the RIOT libraries that are useful for the Air quality project.
You need to edit:
* BOARD (default: b-l072z-lrwan1)
* DRIVER (default: sx1276)
* LORA_REGION (default: EU868)
* DEVEUI, APPEUI and APPKEY

### main.c
the program allows to measure the air quality PM2.5, PM10 (and PM1 only with the pms7003), the temperature, the humidity and sends the datas with the lorawan protocol.

