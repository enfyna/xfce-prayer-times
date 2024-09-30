#include <time.h>

// CONSTANTS
#define PI 3.14159265358979323846
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)
#define J2000_EPOCH 2451545.0

#define DESCEND_CORRECTION 0

// degrees
typedef struct {
    double FAJR;
    double SUNRISE;
    double ASR;
    double MAGHRIB;
    double ISHA;
} sun_altitude_list;

// degrees
typedef struct {
    double FAJR;
    double SUNRISE;
    double ASR;
    double MAGHRIB;
    double ISHA;
} hour_angle_list;

typedef struct {
    int HOUR;
    int MINUTE;
    int SECOND;
} prayer_time;

typedef struct {
    prayer_time* FAJR;
    prayer_time* SUNRISE;
    prayer_time* ASR;
    prayer_time* MAGHRIB;
    prayer_time* ZUHR;
    prayer_time* ISHA;
} prayer_times_list;

char* prayer_time_to_string(prayer_time* time);
prayer_time* double_time_to_time(double time);

prayer_times_list* get_prayer_times_list(struct tm* time, double LONG, double LAT, double elevation, int SF, double FAJR_ANGLE, double ISHA_ANGLE);
prayer_time* get_next_prayer(prayer_times_list* pt_list);
