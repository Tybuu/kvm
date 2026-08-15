#
# Makefile
#

CIRCLEHOME = ../..

OBJS	= main.o kernel.o

LIBS	= $(CIRCLEHOME)/app/lib/template/libtempl.a \
	  $(CIRCLEHOME)/lib/libcircle.a \
		$(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a

include ../Rules.mk

-include $(DEPS)
