#ifndef _H_CALCULATE_TIMES
#define _H_CALCULATE_TIMES

#define PI 3.1415926535897932384626433832795028841971693993751
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)
#define J2000_EPOCH 2451545.0

#define PT_TIME_COUNT 6

#define fajr(v) (v).items[FAJR]
#define sunrise(v) (v).items[SUNRISE]
#define zuhr(v) (v).items[ZUHR]
#define asr(v) (v).items[ASR]
#define maghrib(v) (v).items[MAGHRIB]
#define isha(v) (v).items[ISHA]

#define isha_angle(v) (v)->items[0]
#define fajr_angle(v) (v)->items[1]
#define latitude(v) (v)->items[2]
#define longitude(v) (v)->items[3]
#define shadow_factor(v) (v)->items[4]
#define elevation(v) (v)->items[5]
#define descend_correction(v) (v)->items[6]

typedef enum {
    FAJR,
    SUNRISE,
    ZUHR,
    ASR,
    MAGHRIB,
    ISHA
} PT_Times;

typedef struct {
    char HOUR;
    char MINUTE;
    char SECOND;
} pt_time;

typedef struct {
    char data[9];
} pt_time_cstr;

#define PT_ARGS_COUNT 7

typedef struct {
    // 0: isha_angle
    // 1: fajr_angle;
    // 2: latitude
    // 3: longitude;
    // 4: shadow_factor
    // 5: elevation
    // 6: descend_correction
    double items[PT_ARGS_COUNT];
} pt_args; // find a better name ?

// degrees
typedef struct {
    double items[PT_TIME_COUNT];
} calc_list;

typedef struct {
    pt_time items[PT_TIME_COUNT];
} pt_list;

pt_time_cstr pt_to_string(pt_time time, int show_seconds);
pt_time pt_double_to_time(double time);

pt_list pt_get_list(pt_args* args);
pt_time* pt_next_prayer(pt_list* pt_list);
#endif // _H_CALCULATE_TIMES
