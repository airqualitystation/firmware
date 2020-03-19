# Notes Overview:
This C program allow you to inport and manage the downloaded datas files from Cayenne to your spreadsheet.    

**Terminal input is :** `./exe <file_from_cayenne.txt> <file_to_excel.txt>`  

# Software requirements on Linux :

_Make sure you've updated your distribution with this terminal command `sudo apt-get update`_  

* **GCC compiler** : `sudo apt-get install gcc`  
* **Excel Microsoft Office**
* **LibreOffice Calc**

# Software requirements on Windows :

_If you don't want install a linux distribution on your machine (dualboot or virtual machine), now you can install the wonderful Ubuntu terminal for Windows 10._  
 -> Follow this tutorial : [Tutorial Ubuntu on Windows](https://ubuntu.com/tutorials/tutorial-ubuntu-on-windows#1-overview)  

_Make sure you've updated your distribution with this terminal command `sudo apt-get update`_  

* **GCC compiler** : `sudo apt-get install gcc`  
* **Excel Microsoft Office**
* **LibreOffice Calc**

# Build and run the program :

Please write these next commands in the same order :  

### Build :
`gcc  -Wall  -o datas_format.o -c datas_format.c`  
`gcc -o exe  datas_format.o`  
### Run :
`./exe <file_from_cayenne.txt> <file_to_excel.txt>`  
**Example** : `./exe sensor_datas.txt my_excel_file.txt`  



Now you can oppen and check datas from cayenne in your favorite spreadsheet.
