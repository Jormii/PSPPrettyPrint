MAIN_OBJ = ../main.o
SOURCE_DIR = ../src/
INCLUDE_DIR = "../include/"

TARGET = PSP_TARGET
OBJS =  $(SOURCE_DIR)callbacks.o $(SOURCE_DIR)log_error.o $(SOURCE_DIR)window.o $(SOURCE_DIR)screen.o \
		$(MAIN_OBJ)

LIBS =

INCDIR = $(INCLUDE_DIR)
CFLAGS = -G0 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-rtti -fexceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = EBOOT_TITLE

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
