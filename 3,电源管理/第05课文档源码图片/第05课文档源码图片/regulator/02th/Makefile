KERN_DIR = /work/system/linux-3.4.2

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= regulator.o
obj-m	+= machine.o
obj-m	+= consumer_lcd_4.3.o
