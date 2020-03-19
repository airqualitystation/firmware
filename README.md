# Air Quality Station :: Firmware
This repository contains the RIOT firmware of the Air Quality Station project.
You'll find four folders: _**drivers** , **mainCode** , **examples** and **Cayenne_To_Excel**_.  
* _**drivers**_ contains all the modifications that we have brought in the driver folder of RIOT OS.  
* _**examples**_ contains all the applications test that we have made to study the RIOT features.  
* _**mainCode**_ contains the main application of the project called Air_quality.  
* _**Cayenne_To_Excel**_ contains a short C program to import and manage the downloaded datas files from Cayenne to Excel or an other spreadsheet.  
## Hardware requirements
* [STM32 Nucleo B-L072Z-LRWAN1](https://www.st.com/en/evaluation-tools/b-l072z-lrwan1.html)
* [SDS011](https://cdn-reichelt.de/documents/datenblatt/X200/SDS011-DATASHEET.pdf)
* [PMS7003](https://usermanual.wiki/Document/PMS7003seriesdata20manualEnglishV25.1220636559/view)
* [DHT22](https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf)
* [BME280](https://www.waveshare.com/w/upload/7/75/BME280_Environmental_Sensor_User_Manual_EN.pdf)
* [Solar Panel 180x80 mm](https://www.gotronic.fr/art-cellule-solaire-sol2w-18995.htm#complte_desc)
* [LiPo Battery 3.7 V / 2000 mAh](https://www.sparkfun.com/datasheets/Batteries/UnionBattery-2000mAh.pdf)
* [Solar Power Manager 5V](https://www.dfrobot.com/product-1712.html)

## Build and flash applications
* First, you need to clone, or download, [the RIOT library](https://github.com/RIOT-OS/RIOT.git)
* To build the project, you need to copy the files called "drivers" and "examples" in your RIOT file
* You need to merge and replace files


* Each applications (examples, Cayenne_To_Excel and mainCode folders) have a **_README note_** 

You can compile the applications test and the Air_quality project in your board with the command: `make flash` or `make flash term` if you have pyterm.

## RIOT OS NOTES :

### How to change the standard output UART interface ?

* In **RIOT/sys/include/stdio_uart.h** change this line as you want (according to your board peripheric configuration):  

`#define STDIO_UART_DEV          UART_DEV(0)`

 -> UART_DEV(0) , UART_DEV(1) ... 
### How to change the board Bauderate ? 

* In **RIOT/sys/include/stdio_uart.h** change this line as you want :  

`#define STDIO_UART_BAUDRATE     (115200)`  

 -> 9600, 115200 , 34800 ...  


### How to change the terminal Bauderate with pyterm ? 

* By enter this terminal command to flash and starting pyterm :  

`make flash  term  \ TERMFLAGS=" -p /dev/ttyACM0 -b  your_bauderate"`

* Or just starting the standard output with pyterm :  

`make term  \ TERMFLAGS=" -p /dev/ttyACM0 -b  your_bauderate"` 

### Problem often reported with make command : 

* Ok ... We think you don't have the rights on your serial port. Well try that :  

`sudo chmod o+rw /dev/ttyACM0`

