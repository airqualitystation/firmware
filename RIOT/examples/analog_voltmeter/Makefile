# name of your application
APPLICATION = analog_voltmeter

# If no BOARD is found in the environment, use this default:
#BOARD ?= nucleo-f446re
BOARD ?= b-l072z-lrwan1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += xtimer
USEMODULE += ads101x


CFLAGS += -DDEVELHELP

QUIET ?= 1

include $(RIOTBASE)/Makefile.include
