# name of your application
APPLICATION = Qualite_air

# Use the ST B-L072Z-LRWAN1 board by default:
BOARD ?= b-l072z-lrwan1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

DEVEUI ?= 3131353852378418
APPEUI ?= 70B3D57ED0029E7E
APPKEY ?= DE5B2701879DE3D37A3BC5C49E97D68A

# Default radio driver is Semtech SX1276 (used by the B-L072Z-LRWAN1 board)
DRIVER ?= sx1276

# Default region is Europe and default band is 868MHz
LORA_REGION ?= EU868

# Include the Semtech-loramac package
USEPKG += semtech-loramac
USEPKG += cayenne-lpp

USEMODULE += $(DRIVER)
USEMODULE += fmt
USEMODULE += semtech_loramac_rx
USEMODULE += sds011
USEMODULE += pms7003
USEMODULE += dht
USEMODULE += bme280_i2c
USEMODULE += xtimer

FEATURES_REQUIRED += periph_rtc

CFLAGS += -DDEVEUI=\"$(DEVEUI)\" -DAPPEUI=\"$(APPEUI)\" -DAPPKEY=\"$(APPKEY)\"
CFLAGS += -DBMX280_PARAM_I2C_DEV=I2C_DEV\(0\)
CFLAGS += -DBMX280_PARAM_I2C_ADDR=0x77

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
