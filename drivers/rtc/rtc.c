/**
 * H3OS — MC146818 RTC driver
 */
#include "rtc.h"
#include <h3os/kernel.h>

static u8 cmos_read(u8 reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static u8 bcd_to_bin(u8 v) {
    return (u8)((v & 0x0F) + ((v >> 4) * 10));
}

void rtc_init(void) {
    rtc_time_t t;
    rtc_read(&t);
    KLOG_INFO("rtc", "CMOS time %u-%u-%u %u:%u:%u",
              t.year, t.month, t.day, t.hour, t.minute, t.second);
}

void rtc_read(rtc_time_t* out) {
    while (cmos_read(0x0A) & 0x80) { /* update in progress */ }

    u8 sec = cmos_read(0x00);
    u8 min = cmos_read(0x02);
    u8 hr  = cmos_read(0x04);
    u8 day = cmos_read(0x07);
    u8 mon = cmos_read(0x08);
    u8 yr  = cmos_read(0x09);
    u8 cent = cmos_read(0x32);
    u8 regb = cmos_read(0x0B);

    if (!(regb & 0x04)) { /* BCD */
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hr  = bcd_to_bin(hr);
        day = bcd_to_bin(day);
        mon = bcd_to_bin(mon);
        yr  = bcd_to_bin(yr);
        cent = bcd_to_bin(cent);
    }

    out->second = sec;
    out->minute = min;
    out->hour = hr;
    out->day = day;
    out->month = mon;
    out->year = (u16)(cent * 100 + yr);
    if (out->year < 2000) out->year += 2000;
}
