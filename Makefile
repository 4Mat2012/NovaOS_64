CC = gcc
LD = ld
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIE -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -Iinclude
LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T linker.ld

all: nova.iso

src/kernel/main.o: src/kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: src/kernel/main.o
	$(LD) $(LDFLAGS) src/kernel/main.o -o kernel.elf

# limine.conf yerine limine.cfg kullanıyoruz
nova.iso: kernel.elf limine.cfg
	rm -rf iso_root
	mkdir -p iso_root/boot/limine
	
	# Çekirdeği kopyala
	cp kernel.elf iso_root/
	cp kernel.elf iso_root/boot/
	
	# Yapılandırmayı .cfg olarak kopyala
	cp limine.cfg iso_root/
	cp limine.cfg iso_root/boot/
	cp limine.cfg iso_root/boot/limine/
	
	# Gerekli sistem dosyaları
	cp limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	
	# ISO oluşturma
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o nova.iso
	./limine/limine bios-install nova.iso

run: nova.iso
	qemu-system-x86_64 -cdrom nova.iso -m 512M

# İstediğin clean komutu
clean:
	rm -f nova.iso kernel.elf src/kernel/main.o
	rm -rf iso_root