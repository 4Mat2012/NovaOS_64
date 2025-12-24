#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "font.h"

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

uint32_t* fb_ptr;
uint64_t fb_width, fb_height, fb_pitch;

// --- I/O PORTLARI ---
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    // "=a" yerine "=al" diyemeyiz, ancak 'a' kısıtlayıcısı uint8_t ile AL'yi kullanır.
    __asm__ __volatile__ ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// --- BGA 1080p DRIVER ---
void bga_write(uint16_t index, uint16_t data) {
    outw(0x01CE, index);
    outw(0x01CF, data);
}
void set_1080p_mode() {
    bga_write(4, 0); bga_write(1, 1920); bga_write(2, 1080); bga_write(3, 32); bga_write(4, 1);
}

// --- QWERTZ MAP ---
const char ascii_table[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '?', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'u', '+', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'o', 'a', '^', 0,
    '#', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, '*', 0, ' '
};

// --- GRAFIK ---
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            if (j >= 0 && j < (int)fb_width && i >= 0 && i < (int)fb_height)
                fb_ptr[i * (fb_pitch / 4) + j] = color;
        }
    }
}
void draw_char(char c, int x, int y, uint32_t color) {
    if ((unsigned char)c > 127) return;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (font8x8_basic[(int)c][i] & (0x80 >> j)) {
                if ((uint64_t)(x + j) < fb_width && (uint64_t)(y + i) < fb_height)
                    fb_ptr[(y + i) * (fb_pitch / 4) + (x + j)] = color;
            }
        }
    }
}
void draw_string(const char* str, int x, int y, uint32_t color) {
    while (*str) { draw_char(*str, x, y, color); x += 8; str++; }
}

void draw_window(int x, int y, int w, int h, const char* title, uint32_t border) {
    draw_rect(x, y, w, h, border);
    draw_rect(x + 2, y + 2, w - 4, h - 4, 0x11111b);
    draw_rect(x + 2, y + 2, w - 4, 22, 0x181825);
    draw_string(title, x + 10, y + 7, 0xcdd6f4);
}

// --- UI GÜNCELLEME ---
void update_ui(int gap, int bar_h, int win_w, int win_h, int active_window) {
    draw_window(gap, bar_h + gap, win_w, win_h, "Terminal 1", (active_window == 1 ? 0x89b4fa : 0x585b70));
    draw_window(gap + win_w + gap, bar_h + gap, win_w, win_h, "Terminal 2", (active_window == 2 ? 0x89b4fa : 0x585b70));
}

// --- KERNEL ---
void _start(void) {
    set_1080p_mode();
    if (framebuffer_request.response == NULL) while(1);
    struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    fb_ptr = (uint32_t*)fb->address; fb_width = 1920; fb_height = 1080; fb_pitch = 1920 * 4;

    int gap = 20, bar_h = 35;
    int win_w = (fb_width - (gap * 3)) / 2;
    int win_h = fb_height - bar_h - (gap * 2);
    int active_window = 1;
    int cur1_x = gap + 15, cur1_y = bar_h + gap + 40;
    int cur2_x = gap + win_w + gap + 15, cur2_y = bar_h + gap + 40;

    draw_rect(0, 0, fb_width, fb_height, 0x1e1e2e);
    draw_rect(0, 0, fb_width, bar_h, 0x11111b);
    draw_string("NovaOS | TAB: Focus | Backspace: Delete", 20, 12, 0x89b4fa);

    update_ui(gap, bar_h, win_w, win_h, active_window);

    while(1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            if (!(scancode & 0x80)) { // Tuş basıldıysa
                if (scancode == 0x0F) { // TAB tuşu
                    active_window = (active_window == 1 ? 2 : 1);
                    update_ui(gap, bar_h, win_w, win_h, active_window);
                } 
                else if (scancode == 0x0E) { // BACKSPACE
                    if (active_window == 1 && cur1_x > (gap + 15)) {
                        cur1_x -= 8; 
                        draw_rect(cur1_x, cur1_y, 8, 16, 0x11111b);
                    } else if (active_window == 2 && cur2_x > (gap + win_w + gap + 15)) {
                        cur2_x -= 8; 
                        draw_rect(cur2_x, cur2_y, 8, 16, 0x11111b);
                    }
                } 
                else if (ascii_table[scancode] != 0) {
                    char c = ascii_table[scancode];
                    if (active_window == 1) {
                        draw_char(c, cur1_x, cur1_y, 0xcdd6f4); cur1_x += 8;
                        if (cur1_x > (gap + win_w - 20)) { cur1_x = gap + 15; cur1_y += 16; }
                    } else {
                        draw_char(c, cur2_x, cur2_y, 0xcdd6f4); cur2_x += 8;
                        if (cur2_x > (fb_width - 40)) { cur2_x = gap + win_w + gap + 15; cur2_y += 16; }
                    }
                }
            }
        }
    }
}