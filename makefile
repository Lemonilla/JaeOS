# Makefile for mipsel-linux

SUPDIR = /usr/include/uarm

DEFS = h/const.h h/types.h e/pcb.e e/asl.e e/initial.e e/interrupt.e e/scheduler.e e/exceptions.e $(SUPDIR)/libuarm.h Makefile

CFLAGS =  -mcpu=arm7tdmi -c
LDCOREFLAGS =  -T $(SUPDIR)/ldscripts/elf32ltsarm.h.uarmcore.x

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld

#main target
all: kernel.core.uarm 

kernel.core.uarm: p2test.o initial.o interrupt.o scheduler.o exceptions.o asl.o pcb.o 
	$(LD) $(LDCOREFLAGS) -o kernel.core.uarm initial.o interrupt.o scheduler.o exceptions.o asl.o pcb.o $(SUPDIR)/crtso.o $(SUPDIR)/libuarm.o

p2test.o: phase2/p2test.c $(DEFS)
	$(CC) $(CFLAGS) phase2/p2test.c
 
initial.o: phase2/initial.c $(DEFS)
	$(CC) $(CFLAGS) phase2/initial.c

interrupt.o: phase2/interrupt.c $(DEFS)
	$(CC) $(CFLAGS) phase2/interrupt.c
 
scheduler.o: phase2/scheduler.c $(DEFS)
	$(CC) $(CFLAGS) phase2/scheduler.c

exceptions.o: phase2/exceptions.c $(DEFS)
	$(CC) $(CFLAGS) phase2/exceptions.c
 
asl.o: phase1/asl.c $(DEFS)
	$(CC) $(CFLAGS) phase1/asl.c

pcb.o: phase1/pcb.c $(DEFS)
	$(CC) $(CFLAGS) phase1/pcb.c

# crti.o: crti.s
# 	$(AS) crti.s -o crti.o

clean:
	rm -f *.o *.uarm kernel
clear:
	rm -f *.o *.uarm kernel

distclean: clean
	-rm kernel.*.uarm tape0.uarm