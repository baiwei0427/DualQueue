obj-m +=pias.o
pias-y :=params.o flow.o network.o main.o
EXTRA_CFLAGS += -DANTI_STARVATION

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	