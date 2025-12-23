#include <stdint.h>
#include <stddef.h>
#include "../../limine/limine.h"

// Limine Bootloader'dan framebuffer (ekran bellek alanı) istiyoruz
static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Çekirdeğin giriş noktası
void _start(void) {
    // Ekran kartı yanıt verdi mi kontrol et
    if (fb_req.response == NULL || fb_req.response->framebuffer_count < 1) {
        for (;;) __asm__("hlt");
    }

    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
    uint32_t *fb_ptr = (uint32_t*)fb->address;
    
    // 1. AŞAMA: Gradyan Arka Plan (Görsel Kanıt)
    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            // Matematiksel renk geçişi (x: Kırmızı, y: Yeşil, sabit: Mavi)
            uint8_t r = (uint8_t)((x * 255) / fb->width);
            uint8_t g = (uint8_t)((y * 255) / fb->height);
            uint8_t b = 150; 

            uint32_t color = (r << 16) | (g << 8) | b;

            // Pitch kullanarak pikseli doğru adrese yaz (Donanım uyumluluğu için şart)
            fb_ptr[y * (fb->pitch / 4) + x] = color;
        }
    }

    // 2. AŞAMA: NovaOS "Core" Simgesi (Merkezi Kare)
    int size = 80; 
    int start_x = (fb->width / 2) - (size / 2);
    int start_y = (fb->height / 2) - (size / 2);

    for (int y = start_y; y < start_y + size; y++) {
        for (int x = start_x; x < start_x + size; x++) {
            fb_ptr[y * (fb->pitch / 4) + x] = 0xffffff; // Saf beyaz
        }
    }

    // Bilgisayarı açık tut ama işlemciyi yorma
    for (;;) {
        __asm__("hlt");
    }
}