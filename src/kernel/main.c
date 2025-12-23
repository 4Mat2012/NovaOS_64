#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "font.h" // Font dizisini buraya dahil ediyoruz

// Limine Framebuffer isteği
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Global değişkenler (Çizim kolaylığı için)
uint32_t* fb_ptr;
uint64_t fb_width;
uint64_t fb_height;
uint64_t fb_pitch;

// Temel Piksel Çizme
void put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= fb_width || y < 0 || y >= fb_height) return;
    fb_ptr[y * (fb_pitch / 4) + x] = color;
}

// Dikdörtgen Çizme (GUI için en önemli fonksiyon)
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            put_pixel(j, i, color);
        }
    }
}

// Harf Çizme
void draw_char(char c, int x, int y, uint32_t color) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // Font dizisindeki bitleri kontrol et
            if (font8x8_basic[(int)c][i] & (1 << j)) {
                put_pixel(x + j, y + i, color);
            }
        }
    }
}

// Kelime Yazma
void draw_string(const char* str, int x, int y, uint32_t color) {
    while (*str) {
        draw_char(*str, x, y, color);
        x += 8; // Bir sonraki harfe geç
        str++;
    }
}

// Kernel Giriş Noktası
void _start(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        while(1); 
    }

    struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    fb_ptr = (uint32_t*)fb->address;
    fb_width = fb->width;
    fb_height = fb->height;
    fb_pitch = fb->pitch;

    // 1. Ekranı Koyu Gri Yap (Arka Plan)
    draw_rect(0, 0, fb_width, fb_height, 0x1E1E1E);

    // 2. Mavi Görev Çubuğu (Taskbar) Çiz (Maqs stili)
    draw_rect(0, fb_height - 40, fb_width, 40, 0x005A9E);

    // 3. "Başlat" Butonu Benzeri Bir Kutu
    draw_rect(5, fb_height - 35, 80, 30, 0x0078D4);
    draw_string("NOVA", 25, fb_height - 25, 0xFFFFFF);

    // 4. Hoşgeldin Mesajı
    draw_string("NovaOS v0.1 Online - 1920x1080 Mode", 20, 20, 0x00FF00);
    draw_string("GUI yuklendi. Mouse ve Klavye bekleniyor...", 20, 40, 0xFFFFFF);

    // İşlemciyi durdur
    while(1);
}