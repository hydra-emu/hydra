powerpc-eabi-gcc -DHW_RVL -DGEKKO -mno-eabi -mno-sdata -mcpu=750 -Wall -O2 -ffreestanding -std=gnu99 -c -o build/main.o main.c
powerpc-eabi-gcc -DHW_RVL -DGEKKO -mno-eabi -mno-sdata -mcpu=750 -Wall -O2 -ffreestanding -std=gnu99 -c -o build/string.o string.c
powerpc-eabi-gcc -DHW_RVL -DGEKKO -mno-eabi -mno-sdata -mcpu=750 -Wall -O2 -ffreestanding -std=gnu99 -c -o build/crt0.o crt0.s

(cd build && ${DEVKITPPC}/bin/powerpc-eabi-ld -T ../link.ld -o wii.elf main.o string.o crt0.o)
