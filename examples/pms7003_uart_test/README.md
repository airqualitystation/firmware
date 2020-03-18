# Notes :

This program is a simple test of the UART protocol forthe pms7003 sensor.  
It show all the datas frame receive on the RX board.    
This program is an easy way to understand how the UART communication works.    
* For the synchronization between the RX callback and the main fonction, we use mutex
but a messaging system can be used with the static PID of the main Thread  

## Send command through UART interface at the PMS7003

To do that you need this procedure :  
`void uart_write (uart_t uart, const uint8_t *data, size_t len);'  

For example if you use UART(1) on your board then you can do this for the **PMS7003**:

```
uint8_t cmd_sleep[7] = {0x42 , 0x4d , 0xe4 , 0x00 , 0x00 , 0x01 , 0x73 };
uint8_t cmd_wakeup[7] = {0x42 , 0x4d , 0xe4 , 0x00 , 0x01 , 0x01 , 0x74 };
uint8_t cmd_active[7] = {0x42 , 0x4d , 0xe1 , 0x00 , 0x01 , 0x01 , 0x71 };
uint8_t cmd_passive[7] = {0x42 , 0x4d , 0xe1 , 0x00 , 0x00 , 0x01 , 0x70 };
uint8_t cmd_read[7] = {0x42 , 0x4d , 0xe2 , 0x00 , 0x00 , 0x01 , 0x71 };

```

Then add this line if you want send a wakeup command:  

`uart_write(UART(1), cmd_wakeup,7);`


