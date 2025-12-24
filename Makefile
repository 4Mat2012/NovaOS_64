CC = gcc
LD = ld
OBJ = src/kernel/main.o src/kernel/serial.o
# Bayrakları temizleyelim ve düzene sokalım
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding \
         -fno-stack-protector -fno-stack-check -fno-lto -fno-pie \
         -m64 -march=x86-64 \
         -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
         -mno-red-zone \
         -mcmodel=kernel \
         -Iinclude

# Linker bayrakları - Yeni linker uyarısını kapatmak için flag ekledim
LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 \
          -T linker.ld --no-warn-rwx-segments

		  

all: nova.iso

src/kernel/main.o: src/kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o kernel.elf
# ^-- Burası mutlaka 1 adet TAB karakteri olmalı, boşluk değil!

nova.iso: kernel.elf limine.cfg
	rm -rf iso_root
	mkdir -p iso_root/boot/limine
	cp kernel.elf iso_root/boot/
	cp limine.cfg iso_root/boot/limine/
	cp limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o nova.iso
	./limine/limine bios-install nova.iso

run: nova.iso
	qemu-system-x86_64 -cdrom nova.iso -vga std -m 1G -vga std -global VGA.vgamem_mb=128

clean:
	rm -f nova.iso kernel.elf src/kernel/main.o
	rm -rf iso_root