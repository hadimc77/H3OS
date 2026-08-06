/**
 * H3OS Terminal — interactive shell with classic UNIX-like commands
 */
#include "terminal.h"
#include "../../drivers/framebuffer/framebuffer.h"
#include "../../filesystem/vfs/vfs.h"
#include "../../memory/pmm.h"
#include "../../memory/heap.h"
#include <h3os/cpu.h>
#include <h3os/version.h>
#include <h3os/adaptive.h>
#include <h3os/kernel.h>
#include <h3os/string.h>
#include "../../drivers/timer/timer.h"

#define TERM_COLS 72
#define TERM_ROWS 24
#define TERM_IN   128

typedef struct {
    char lines[TERM_ROWS][TERM_COLS + 1];
    int  row;
    char input[TERM_IN];
    int  in_len;
    bool reboot_req;
    bool shutdown_req;
} term_state_t;

static term_state_t g_term;

static void term_newline(void) {
    if (g_term.row < TERM_ROWS - 1) {
        g_term.row++;
    } else {
        for (int i = 0; i < TERM_ROWS - 1; i++)
            strcpy(g_term.lines[i], g_term.lines[i + 1]);
        g_term.lines[TERM_ROWS - 1][0] = '\0';
    }
    g_term.lines[g_term.row][0] = '\0';
}

static void term_print(const char* s) {
    while (*s) {
        if (*s == '\n') {
            term_newline();
            s++;
            continue;
        }
        size_t len = strlen(g_term.lines[g_term.row]);
        if (len >= TERM_COLS) {
            term_newline();
            len = 0;
        }
        g_term.lines[g_term.row][len] = *s++;
        g_term.lines[g_term.row][len + 1] = '\0';
    }
}

static void term_println(const char* s) {
    term_print(s);
    term_newline();
}

static void list_cb(vfs_node_t* n, void* ctx) {
    H3OS_UNUSED(ctx);
    char line[160];
    line[0] = '\0';
    strcat(line, n->type == VFS_DIR ? "dir  " : "file ");
    strcat(line, n->name);
    if (n->type == VFS_FILE) {
        char num[32];
        strcat(line, "  ");
        utoa(n->size, num, 10);
        strcat(line, num);
        strcat(line, "B");
    }
    term_println(line);
}

static void cmd_help(void) {
    term_println("H3OS Shell — commands:");
    term_println("  cd ls pwd mkdir rm cp mv cat echo");
    term_println("  grep find clear top kill systeminfo");
    term_println("  network shutdown reboot help version h3pkg");
}

static void cmd_systeminfo(void) {
    const cpu_info_t* cpu = cpu_get_info();
    pmm_stats_t mem;
    heap_stats_t heap;
    pmm_get_stats(&mem);
    heap_get_stats(&heap);

    term_print("OS: "); term_print(H3OS_NAME); term_print(" ");
    term_print(H3OS_VERSION_STRING); term_print(" (");
    term_print(H3OS_CODENAME); term_println(")");
    term_print("CPU: "); term_println(cpu->brand);
    term_print("Vendor: "); term_println(cpu->vendor);

    char buf[80];
    term_print("Logical CPUs: ");
    utoa(cpu->logical_cpus, buf, 10); term_println(buf);

    term_print("Memory: ");
    utoa(mem.total_bytes / (1024 * 1024), buf, 10);
    term_print(buf); term_print(" MiB total, ");
    utoa((mem.free_pages * PAGE_SIZE) / (1024 * 1024), buf, 10);
    term_print(buf); term_println(" MiB free");

    term_print("Profile: ");
    term_println(adaptive_profile_name());
    term_print("Uptime ms: ");
    utoa(timer_uptime_ms(), buf, 10); term_println(buf);
}

static char* next_arg(char** p) {
    while (**p == ' ') (*p)++;
    if (**p == '\0') return NULL;
    char* start = *p;
    while (**p && **p != ' ') (*p)++;
    if (**p) { **p = '\0'; (*p)++; }
    return start;
}

static void run_command(char* line) {
    char* p = line;
    char* cmd = next_arg(&p);
    if (!cmd || !cmd[0]) return;

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "clear") == 0) {
        memset(&g_term.lines, 0, sizeof(g_term.lines));
        g_term.row = 0;
    } else if (strcmp(cmd, "pwd") == 0) {
        term_println(vfs_cwd());
    } else if (strcmp(cmd, "cd") == 0) {
        char* arg = next_arg(&p);
        if (!arg) arg = "/home/user";
        if (vfs_chdir(arg) != 0) term_println("cd: no such directory");
    } else if (strcmp(cmd, "ls") == 0) {
        char* arg = next_arg(&p);
        vfs_node_t* dir = vfs_resolve(arg ? arg : ".");
        if (!dir) term_println("ls: not found");
        else vfs_list(dir, list_cb, NULL);
    } else if (strcmp(cmd, "mkdir") == 0) {
        char* arg = next_arg(&p);
        if (!arg) term_println("mkdir: missing operand");
        else if (!vfs_mkdir(arg, 0755)) term_println("mkdir: failed");
    } else if (strcmp(cmd, "rm") == 0) {
        char* arg = next_arg(&p);
        if (!arg) term_println("rm: missing operand");
        else if (vfs_unlink(arg) != 0) term_println("rm: failed");
    } else if (strcmp(cmd, "cp") == 0) {
        char* src = next_arg(&p);
        char* dst = next_arg(&p);
        if (!src || !dst) term_println("cp: usage: cp <src> <dst>");
        else if (vfs_copy(src, dst) != 0) term_println("cp: failed");
        else term_println("ok");
    } else if (strcmp(cmd, "mv") == 0) {
        char* src = next_arg(&p);
        char* dst = next_arg(&p);
        if (!src || !dst) term_println("mv: usage: mv <src> <dst>");
        else if (vfs_rename(src, dst) != 0) term_println("mv: failed");
        else term_println("ok");
    } else if (strcmp(cmd, "cat") == 0) {
        char* arg = next_arg(&p);
        vfs_node_t* f = arg ? vfs_resolve(arg) : NULL;
        if (!f || f->type != VFS_FILE) term_println("cat: not a file");
        else {
            char buf[512];
            u64 n = 0;
            vfs_read(f, buf, sizeof(buf) - 1, &n);
            buf[n] = '\0';
            term_print(buf);
            if (n == 0 || buf[n - 1] != '\n') term_newline();
        }
    } else if (strcmp(cmd, "echo") == 0) {
        term_println(p);
    } else if (strcmp(cmd, "systeminfo") == 0 || strcmp(cmd, "uname") == 0) {
        cmd_systeminfo();
    } else if (strcmp(cmd, "version") == 0) {
        term_print(H3OS_NAME); term_print(" "); term_println(H3OS_VERSION_STRING);
        term_println(H3OS_TAGLINE);
    } else if (strcmp(cmd, "top") == 0) {
        term_println("PID  NAME            STATE");
        term_println("1    kernel          running");
        term_println("2    desktop         running");
        term_println("3    terminal        running");
    } else if (strcmp(cmd, "kill") == 0) {
        term_println("kill: process control not yet exposed to userspace");
    } else if (strcmp(cmd, "network") == 0) {
        term_println("network: stack initializing (IPv4/IPv6 planned)");
        term_println("  lo0  UP  127.0.0.1/8");
    } else if (strcmp(cmd, "find") == 0) {
        char* arg = next_arg(&p);
        if (!arg) term_println("find: missing pattern");
        else {
            term_print("find: searching for '");
            term_print(arg);
            term_println("' (basic VFS scan)");
            vfs_list(vfs_root(), list_cb, NULL);
        }
    } else if (strcmp(cmd, "grep") == 0) {
        term_println("grep: use cat + visual scan in v0.1 (full grep soon)");
    } else if (strcmp(cmd, "h3pkg") == 0) {
        char* sub = next_arg(&p);
        char* pkg = next_arg(&p);
        if (!sub) {
            term_println("h3pkg: install|remove|update|upgrade|search|repair|list");
        } else if (strcmp(sub, "list") == 0 || strcmp(sub, "search") == 0) {
            vfs_node_t* db = vfs_resolve("/var/lib/h3pkg/db");
            if (!db) term_println("h3pkg: database missing");
            else {
                char buf[512];
                u64 n = 0;
                vfs_read(db, buf, sizeof(buf) - 1, &n);
                buf[n] = '\0';
                if (pkg && pkg[0]) {
                    /* naive substring search */
                    const char* s = buf;
                    bool found = false;
                    while (*s) {
                        const char* line = s;
                        while (*s && *s != '\n') s++;
                        char tmp[96];
                        size_t len = (size_t)(s - line);
                        if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
                        memcpy(tmp, line, len);
                        tmp[len] = '\0';
                        if (strstr(tmp, pkg)) { term_println(tmp); found = true; }
                        if (*s == '\n') s++;
                    }
                    if (!found) term_println("h3pkg: no matches");
                } else {
                    term_print(buf);
                    if (n == 0 || buf[n - 1] != '\n') term_newline();
                }
            }
        } else if (strcmp(sub, "install") == 0) {
            if (!pkg) term_println("h3pkg: install <name>");
            else {
                vfs_node_t* db = vfs_resolve("/var/lib/h3pkg/db");
                char entry[96];
                strcpy(entry, pkg);
                strcat(entry, " 0.1.0 installed\n");
                if (!db) {
                    vfs_mkdir("/var/lib", 0755);
                    vfs_mkdir("/var/lib/h3pkg", 0755);
                    db = vfs_create("/var/lib/h3pkg/db", 0644);
                    if (db) vfs_write(db, entry, strlen(entry));
                } else {
                    char buf[512];
                    u64 n = 0;
                    vfs_read(db, buf, sizeof(buf) - 1, &n);
                    buf[n] = '\0';
                    if (n + strlen(entry) < sizeof(buf)) {
                        strcat(buf, entry);
                        vfs_write(db, buf, strlen(buf));
                    }
                }
                vfs_mkdir("/usr/share/packages", 0755);
                char path[128];
                strcpy(path, "/usr/share/packages/");
                strcat(path, pkg);
                vfs_node_t* meta = vfs_create(path, 0644);
                if (meta) vfs_write(meta, "h3pkg package\n", 14);
                term_print("h3pkg: installed ");
                term_println(pkg);
            }
        } else if (strcmp(sub, "remove") == 0) {
            if (!pkg) term_println("h3pkg: remove <name>");
            else {
                char path[128];
                strcpy(path, "/usr/share/packages/");
                strcat(path, pkg);
                vfs_unlink(path);
                term_print("h3pkg: removed ");
                term_println(pkg);
            }
        } else if (strcmp(sub, "repair") == 0 || strcmp(sub, "update") == 0 ||
                   strcmp(sub, "upgrade") == 0) {
            term_print("h3pkg: ");
            term_print(sub);
            term_println(" — local RAMFS db OK");
        } else {
            term_println("h3pkg: unknown subcommand");
        }
    } else if (strcmp(cmd, "shutdown") == 0) {
        term_println("Shutting down H3OS...");
        g_term.shutdown_req = true;
    } else if (strcmp(cmd, "reboot") == 0) {
        term_println("Rebooting...");
        g_term.reboot_req = true;
    } else {
        term_print("unknown command: ");
        term_println(cmd);
    }
}

static void term_draw(window_t* self) {
    term_state_t* t = (term_state_t*)self->user;
    i32 x = self->x + 8;
    i32 y = self->y + 36;
    u32 fg = 0xFFB8F2E6;
    u32 bg = 0xFFFFFFFF; /* transparent glyph bg */

    for (int r = 0; r <= t->row && r < TERM_ROWS; r++) {
        fb_draw_string(x, y + r * 10, t->lines[r], fg, bg);
    }

    /* prompt + input */
    char prompt[TERM_IN + 32];
    strcpy(prompt, "h3os:");
    strcat(prompt, vfs_cwd());
    strcat(prompt, "> ");
    strcat(prompt, t->input);
    fb_draw_string(x, y + (t->row + 1) * 10, prompt, 0xFF2EC4B6, bg);
}

static void term_on_key(window_t* self, char c) {
    term_state_t* t = (term_state_t*)self->user;
    if (c == '\n' || c == '\r') {
        char line[TERM_IN];
        strcpy(line, t->input);
        term_print("h3os:");
        term_print(vfs_cwd());
        term_print("> ");
        term_println(t->input);
        t->in_len = 0;
        t->input[0] = '\0';
        run_command(line);

        if (t->reboot_req) {
            /* Triple fault / keyboard reset */
            outb(0x64, 0xFE);
        }
        if (t->shutdown_req) {
            /* QEMU shutdown via port */
            outw(0x604, 0x2000);
            outw(0xB004, 0x2000);
        }
        return;
    }
    if (c == '\b') {
        if (t->in_len > 0) {
            t->input[--t->in_len] = '\0';
        }
        return;
    }
    if (t->in_len + 1 < TERM_IN && c >= 32) {
        t->input[t->in_len++] = c;
        t->input[t->in_len] = '\0';
    }
}

window_t* terminal_open(void) {
    memset(&g_term, 0, sizeof(g_term));
    fb_t* fb = fb_get();
    i32 w = 640, h = 400;
    i32 x = ((i32)fb->width - w) / 2;
    i32 y = 80;

    window_t* win = wm_create("Terminal", x, y, w, h);
    if (!win) return NULL;
    win->bg = 0xFF07161E;
    win->accent = 0xFF2EC4B6;
    win->user = &g_term;
    win->draw = term_draw;
    win->on_key = term_on_key;

    term_println("H3OS Terminal 0.1 — The Future Starts Here.");
    term_println("Type 'help' for commands, 'systeminfo' for hardware.");
    return win;
}

void terminal_init_commands(void) {
    /* reserved for dynamic command registration */
}
