MAIN_OBJ = ../main.o
SOURCE_DIR = ../src/
NON_LIBRARY_SOURCE_DIR = ../non-library_src/
INCLUDE_DIR = ../include/ ../non-library_include/

TARGET = PSP_TARGET
OBJS =  $(NON_LIBRARY_SOURCE_DIR)callbacks.o \
		$(SOURCE_DIR)log_error.o $(SOURCE_DIR)base_set_font.o \
		$(SOURCE_DIR)screen_buffer.o \
		$(SOURCE_DIR)text_buffer.o \
		$(SOURCE_DIR)window.o $(SOURCE_DIR)window_traversal.o \
		$(SOURCE_DIR)margin_display.o $(SOURCE_DIR)window_display.o \
		$(SOURCE_DIR)scrollbar_display.o \
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
