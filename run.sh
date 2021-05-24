as --32 boot.s -o boot.o
gcc -m32 -c ata.c -o ata.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c utils.c -o utils.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c char.c -o char.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

ld -m elf_i386 -T linker.ld kernel.o utils.o char.o boot.o -o MyOS.bin -nostdlib
grub-file --is-x86-multiboot MyOS.bin
mkdir -p isodir/boot/grub
cp MyOS.bin isodir/boot/MyOS.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o ExOS.iso isodir
qemu ExOS.iso
