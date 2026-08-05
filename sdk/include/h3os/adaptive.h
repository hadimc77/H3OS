/**
 * H3OS — Adaptive performance profiles
 */
#ifndef H3OS_ADAPTIVE_H
#define H3OS_ADAPTIVE_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PERF_LOW = 0,
    PERF_BALANCED,
    PERF_HIGH
} perf_profile_t;

typedef struct {
    perf_profile_t profile;
    bool animations;
    bool blur;
    bool shadows;
    bool transparency;
    u32  target_fps;
    u32  max_windows;
} perf_settings_t;

void               adaptive_init(void);
const perf_settings_t* adaptive_settings(void);
const char*        adaptive_profile_name(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_ADAPTIVE_H */
