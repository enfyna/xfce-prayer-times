#ifndef _H_CALCULATE_TIMES
#define _H_CALCULATE_TIMES

#define PI 3.1415926535897932384626433832795028841971693993751
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)
#define J2000_EPOCH 2451545.0

// TODO:
// Add to pt_args
#define DESCEND_CORRECTION 0

#define PT_TIME_COUNT 6

typedef enum {
    FAJR,
    SUNRISE,
    ZUHR,
    ASR,
    MAGHRIB,
    ISHA
} PT_Times;

typedef struct {
    int HOUR;
    int MINUTE;
    int SECOND;
} pt_time;

typedef struct {
    double fajr_angle;
    double isha_angle;
    double latitude;
    double longitude;
    double elevation;
    double shadow_factor;
} pt_args; // find a better name ?

// degrees
typedef struct {
    double items[PT_TIME_COUNT];
} calc_list;

typedef struct {
    pt_time* items[PT_TIME_COUNT];
} pt_list;

char* pt_to_string(pt_time* time);
pt_time* pt_double_to_time(double time);

pt_list* pt_get_list(pt_args* args);
pt_time* pt_next_prayer(pt_list* pt_list);
#endif // _H_CALCULATE_TIMES
