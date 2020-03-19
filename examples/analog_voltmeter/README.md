# Notes :
This application allow you to measure the current and to voltage through the ADS1115. In **hardware** repository you will find the coresponding schematic.  

We wanted to determine the level of battery in real time with **LoRa** thanks the voltage. Moreover we wanted to follow the power consuption with the current.  


The `A0` input is connected to the battery positive output. Then the `A1` input is conected in serial with an 1 Ohm precision resistor.  

This program make the diferential calcul between `A0` and `A1`  

# Why an analog digital converter ?

For the LoRa board project we use, there is no implementation of ADC driver on RIOT OS. But also usually ADC are only 12 bits converter on STM32. This ADS1115 has 16 bits and it even includes a programmable gain amplifier, up to x16, to help boost up smaller single/differential signals to the full range.  

The higher gain allows us to measure an input range of +/-0.256V with 16 bit resolution it's just great for our purpose to monitoring small voltage drops.  

**Find more details in the doc repository**

