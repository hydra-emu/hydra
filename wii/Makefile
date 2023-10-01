CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc
CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++
LD = $(DEVKITPPC)/bin/powerpc-eabi-ld

OUTDIR = build
PROJECT = hydra_wii
DOL_FILE = $(PROJECT).dol
ELF_FILE = $(PROJECT).elf
OBJS = $(OUTDIR)/main.o $(OUTDIR)/string.o $(OUTDIR)/crt0.o
CFLAGS = -DHW_RVL -DGEKKO -mno-eabi -mno-sdata -mcpu=750 -Wall -O2 -ffreestanding -std=gnu99 

all: check-env dir $(OUTDIR)/$(DOL_FILE)

dir:
	mkdir -p $(OUTDIR)

check-env:
ifndef DEVKITPPC
	$(error DEVKITPPC is undefined)
endif

$(OUTDIR)/$(DOL_FILE): $(OUTDIR)/$(ELF_FILE)
	elf2dol $< $@

$(OUTDIR)/$(ELF_FILE): $(OBJS)
	echo $(shell pwd)
	$(LD) -T link.ld -o $@ $^

$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean check-env dir