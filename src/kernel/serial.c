#include <stdint.h>
#include "serial.h"

#define PORT 0x3f8

// outb fonksiyonu (Eger io.h dosyan yoksa bunu buraya ekliyoruz)
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

// inb fonksiyonu
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void init_serial() {
    outb(PORT + 1, 0x00);    // Interruptları kapat
    outb(PORT + 3, 0x80);    // DLAB aç
    outb(PORT + 0, 0x03);    // 38400 baud
    outb(PORT + 1, 0x00);
    outb(PORT + 3, 0x03);    // 8 bit, no parity, 1 stop bit
    outb(PORT + 2, 0xC7);    // FIFO
}

int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);
   outb(PORT, a);
}

void write_serial_str(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        write_serial(str[i]);
    }
}