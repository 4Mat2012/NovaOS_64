#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "font.h"

// Prototipler
void draw_char(struct limine_framebuffer *fb, uint8_t c, int x, int y, uint32_t color);
void draw_string(struct limine_framebuffer *fb, const char *str, int x, int y, uint32_t color);

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void _start(void) {
    if (fb_req.response == NULL || fb_req.response->framebuffer_count < 1) {
        for (;;);
    }

    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
    uint32_t *fb_ptr = (uint32_t *)fb->address;

    // Arka plan: Gradyan
    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            uint32_t b = (x * 255) / fb->width;
            uint32_t g = (y * 255) / fb->height;
            fb_ptr[y * (fb->pitch / 4) + x] = (g << 8) | b;
        }
    }

    // Yazıları buraya basıyoruz
    draw_string(fb, "NovaOS_64", 100, 100, 0xFFFFFFFF);
    draw_string(fb, "by Turker", 100, 115, 0x00FF00);

    for (;;);
}

void draw_char(struct limine_framebuffer *fb, uint8_t c, int x, int y, uint32_t color) {
    uint32_t *fb_ptr = (uint32_t *)fb->address;
    
    // ASCII kontrolü (128 karakterlik font haritamız olduğu için)
    if (c >= 128) return;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font8x8[c][row] >> (7 - col)) & 1) {
                fb_ptr[(y + row) * (fb->pitch / 4) + (x + col)] = color;
            }
        }
    }
}

void draw_string(struct limine_framebuffer *fb, const char *str, int x, int y, uint32_t color) {
    for (int i = 0; str[i] != '\0'; i++) {
        draw_char(fb, (uint8_t)str[i], x + (i * 9), y, color);
    }
}