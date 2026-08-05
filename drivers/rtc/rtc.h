/**
 * H3OS — CMOS RTC clock
 */
#ifndef H3OS_RTC_H
#define H3OS_RTC_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u8 second, minute, hour;
    u8 day, month;
    u16 year;
} rtc_time_t;

void rtc_init(void);
void rtc_read(rtc_time_t* out);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_RTC_H */
