This folder contain all the modifications that we have done on the <driver> folder of RIOT OS.

We have developed a library to manage the pms7003 sensor on RIOT. 
If you want to import it on yout RIOT environement, copy and paste <pms7003> on the coresponding folder on RIOT.
Also don't forget to add the "Makefile.dep" and "Makefile.include" in the <driver> root folder of RIOT.
	
In file <firmware/pms7003/include/pms7003_params.h> you can modify the GPIO for the "set pin" and the UART port (RX/TX).
By default UART_DEV(1) and GPIO_PIN(PORT_A, 8) is used. 


Below, you've a look at the Arduino pin map available for the b-l072z-lrwan1 board: 

*D0 =>	GPIO_PIN(PORT_A, 3)
*D1 =>	GPIO_PIN(PORT_A, 2)
*D2 =>	GPIO_PIN(PORT_A, 10)
*D3 =>	GPIO_PIN(PORT_B, 13)
*D4	=>	GPIO_PIN(PORT_B, 5)
*D5	=>	GPIO_PIN(PORT_B, 7)
*D6	=>	GPIO_PIN(PORT_B, 2)
*D7	=>	GPIO_PIN(PORT_A, 8)
*D8	=>	GPIO_PIN(PORT_A, 9)
*D9	=>	GPIO_PIN(PORT_B, 12)
*D10	=>	GPIO_PIN(PORT_B, 6)
*D11	=>	GPIO_PIN(PORT_B, 15)
*D12	=>	GPIO_PIN(PORT_B, 14)
*D14	=>	GPIO_PIN(PORT_B, 9)
*D15	=>  	GPIO_PIN(PORT_B, 8)

The analog pin are not available because this board doesn't have ADC driver implementation on RIOT


Here a look at the RIOT UART pin map available for this board : 

*UART(0) =>  { RX = GPIO_PIN(PORT_A, 3) ; TX = GPIO_PIN(PORT_A, 2) }
*UART(1)	=>  { RX = GPIO_PIN(PORT_A, 10) ; TX = GPIO_PIN(PORT_A, 9) }

*UART(0) is used by default for the standard output (StLink) 


*Also see our driver test in folder <firmware/examples/pms7003_test>
