/**
 * H3OS — PS/2 Keyboard (scancode set 1, US QWERTY)
 */
#include "keyboard.h"
#include <h3os/interrupts.h>
#include <h3os/kernel.h>

#define KBD_BUF_SIZE 256

static volatile char kbd_buf[KBD_BUF_SIZE];
static volatile u32 kbd_head = 0, kbd_tail = 0;
static bool shift = false, caps = false, ctrl = false;

static const char scancode_map[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0, '\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ', 0,
};

static const char scancode_map_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','"','~', 0, '|',
    'Z','X','C','V','B','N','M','<','>','?', 0, '*', 0, ' ', 0,
};

static void kbd_push(char c) {
    u32 next = (kbd_head + 1) % KBD_BUF_SIZE;
    if (next == kbd_tail) return;
    kbd_buf[kbd_head] = c;
    kbd_head = next;
}

static void keyboard_irq(interrupt_frame_t* frame) {
    H3OS_UNUSED(frame);
    u8 sc = inb(0x60);

    if (sc == 0x1D) { ctrl = true; return; }
    if (sc == 0x9D) { ctrl = false; return; }
    if (sc == 0x2A || sc == 0x36) { shift = true; return; }
    if (sc == 0xAA || sc == 0xB6) { shift = false; return; }
    if (sc == 0x3A) { caps = !caps; return; }
    if (sc & 0x80) return;

    char c = 0;
    if (sc < 128) {
        bool upper = shift ^ caps;
        c = upper ? scancode_map_shift[sc] : scancode_map[sc];
        if (!shift && caps && c >= 'a' && c <= 'z') c = (char)(c - 32);
        if (shift && caps && c >= 'A' && c <= 'Z') c = (char)(c + 32);
    }

    if (!c) return;

    if (ctrl) {
        /* Map Ctrl+Letter to ASCII control codes (Ctrl+A=1 ... Ctrl+Z=26) */
        char base = c;
        if (base >= 'A' && base <= 'Z') base = (char)(base - 'A' + 'a');
        if (base >= 'a' && base <= 'z') {
            kbd_push((char)(base - 'a' + 1));
            return;
        }
    }

    kbd_push(c);
}

void keyboard_init(void) {
    kbd_head = kbd_tail = 0;
    shift = caps = ctrl = false;
    irq_install(1, keyboard_irq);
    KLOG_INFO("kbd", "PS/2 keyboard ready (Ctrl+T terminal, Ctrl+L launcher)");
}

bool keyboard_has_char(void) {
    return kbd_head != kbd_tail;
}

bool keyboard_try_read(char* out) {
    if (kbd_head == kbd_tail) return false;
    *out = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
    return true;
}

char keyboard_read_char(void) {
    char c;
    if (!keyboard_try_read(&c)) return 0;
    return c;
}
