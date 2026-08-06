/**
 * H3OS — PS/2 mouse (auxiliary device on 8042 controller)
 */
#include "mouse.h"
#include "../framebuffer/framebuffer.h"
#include <h3os/interrupts.h>
#include <h3os/kernel.h>
#include <h3os/string.h>

static mouse_state_t g_mouse;
static bool prev_left = false;
static bool prev_right = false;
static u8  cycle = 0;
static u8  packet[3];

static void mouse_wait_write(void) {
    for (int i = 0; i < 100000; i++) {
        if ((inb(0x64) & 2) == 0) return;
    }
}

static void mouse_wait_read(void) {
    for (int i = 0; i < 100000; i++) {
        if (inb(0x64) & 1) return;
    }
}

static void mouse_write(u8 val) {
    mouse_wait_write();
    outb(0x64, 0xD4);
    mouse_wait_write();
    outb(0x60, val);
}

static u8 mouse_read(void) {
    mouse_wait_read();
    return inb(0x60);
}

static void mouse_irq(interrupt_frame_t* frame) {
    H3OS_UNUSED(frame);
    u8 data = inb(0x60);

    if (cycle == 0 && (data & 0x08) == 0) return;
    packet[cycle++] = data;
    if (cycle < 3) return;
    cycle = 0;

    i32 dx = (i32)(i8)packet[1];
    i32 dy = -(i32)(i8)packet[2];
    if (packet[0] & 0xC0) return;

    g_mouse.dx = dx;
    g_mouse.dy = dy;
    g_mouse.x += dx;
    g_mouse.y += dy;

    fb_t* fb = fb_get();
    if (fb && fb->width && fb->height) {
        if (g_mouse.x < 0) g_mouse.x = 0;
        if (g_mouse.y < 0) g_mouse.y = 0;
        if (g_mouse.x >= (i32)fb->width) g_mouse.x = (i32)fb->width - 1;
        if (g_mouse.y >= (i32)fb->height) g_mouse.y = (i32)fb->height - 1;
    }

    bool left = (packet[0] & 1) != 0;
    bool right = (packet[0] & 2) != 0;
    g_mouse.middle = (packet[0] & 4) != 0;

    if (left && !prev_left) g_mouse.left_pressed = true;
    if (!left && prev_left) g_mouse.left_released = true;
    if (right && !prev_right) g_mouse.right_pressed = true;
    if (!right && prev_right) g_mouse.right_released = true;
    g_mouse.left = left;
    g_mouse.right = right;
    prev_left = left;
    prev_right = right;
}

void mouse_init(void) {
    memset(&g_mouse, 0, sizeof(g_mouse));
    g_mouse.x = 640;
    g_mouse.y = 360;
    cycle = 0;
    prev_left = prev_right = false;

    mouse_wait_write();
    outb(0x64, 0xA8);

    mouse_wait_write();
    outb(0x64, 0x20);
    mouse_wait_read();
    u8 status = inb(0x60);
    status |= 0x02;
    status &= (u8)~0x20;
    mouse_wait_write();
    outb(0x64, 0x60);
    mouse_wait_write();
    outb(0x60, status);

    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();

    irq_install(12, mouse_irq);
    KLOG_INFO("mouse", "PS/2 mouse ready (IRQ12)");
}

void mouse_get(mouse_state_t* out) {
    if (out) *out = g_mouse;
}

void mouse_poll_frame(void) {
    g_mouse.left_pressed = false;
    g_mouse.left_released = false;
    g_mouse.right_pressed = false;
    g_mouse.right_released = false;
    g_mouse.dx = 0;
    g_mouse.dy = 0;
}

void mouse_draw_cursor(void) {
    i32 x = g_mouse.x;
    i32 y = g_mouse.y;
    u32 fg = 0xFFF5FBFD;
    u32 tip = 0xFF2EC4B6;
    for (i32 i = 0; i < 14; i++) {
        fb_put_pixel(x, y + i, tip);
        for (i32 j = 0; j <= (i < 10 ? i : 14 - i) && j < 9; j++) {
            fb_put_pixel(x + j, y + i, fg);
        }
    }
    fb_draw_line(x, y, x + 5, y + 12, 0xFF061018);
}
