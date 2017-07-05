
CROSS_COMPILE = arm-linux-
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -Wall -Werror -O2 -g
CFLAGS += -I $(shell pwd)/include

LDFLAGS := -Tsound.lds 

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := sound


obj-y += head.o
obj-y += init.o
obj-y += interrupt.o
obj-y += main.o
obj-y += serial.o
obj-y += codec/
obj-y += format/
obj-y += lib/
obj-y += machine/
obj-y += soc/
all : 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(LD) $(LDFLAGS) -o $(TARGET) built-in.o
	${OBJCOPY} -O binary -S $(TARGET) $(TARGET).bin
	${OBJDUMP} -D -m arm $(TARGET) > $(TARGET).dis


clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
	