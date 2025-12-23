CC ?= cc
STRIP ?= strip
INSTALL ?= ./install-sh

PREFIX ?= /usr/local
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding \
         -fno-stack-protector -fno-stack-check -fno-lto -fno-PIE \
         -m64 -march=x86-64 \
         -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
         -mcmodel=kernel \
         -Iinclude

.PHONY: all
all: limine

.PHONY: install
install: all
	$(INSTALL) -d '$(DESTDIR)$(PREFIX)/share'
	$(INSTALL) -d '$(DESTDIR)$(PREFIX)/share/limine'
	$(INSTALL) -m 644 limine-bios.sys '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 limine-bios-cd.bin '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 limine-uefi-cd.bin '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 limine-bios-pxe.bin '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 BOOTX64.EFI '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 BOOTIA32.EFI '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 BOOTAA64.EFI '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -m 644 BOOTRISCV64.EFI '$(DESTDIR)$(PREFIX)/share/limine/'
	$(INSTALL) -d '$(DESTDIR)$(PREFIX)/include'
	$(INSTALL) -m 644 limine.h '$(DESTDIR)$(PREFIX)/include/'
	$(INSTALL) -d '$(DESTDIR)$(PREFIX)/bin'
	$(INSTALL) limine '$(DESTDIR)$(PREFIX)/bin/'

.PHONY: install-strip
install-strip: install
	$(STRIP) '$(DESTDIR)$(PREFIX)/bin/limine'

.PHONY: clean
clean:
	rm -f limine limine.exe

limine: limine.c
	$(CC) $(CFLAGS) -Wall -Wextra $(WERROR_FLAG) $(CPPFLAGS) $(LDFLAGS) -std=c99 limine.c $(LIBS) -o $@
