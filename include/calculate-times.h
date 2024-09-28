// CONSTANTS
#include <time.h>
#define PI 3.14159265358979323846
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)
#define J2000_EPOCH 2451545.0


#define FAJR_ANGLE 18.0 // diyanet 
#define ISHA_ANGLE 17.0 // diyanet
// #define MAGHRIB_ANGLE -0.833 // Maghrib solar dip below horizon
// #define SF 2 // shadow factor 
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
    double FAJR;
    double SUNRISE;
    double ASR;
    double MAGHRIB;
    double ZUHR;
    double ISHA;
} prayer_times_list;


double calculate_julian_days(int Y, int M, int D, int H, int m, int s, int Z);
double calculate_sun_declination(double jd);
double calculate_equation_of_time(double jd);
double calculate_transit_time(double LONGTITUDE, double ET, double Z);

sun_altitude_list* calculate_sun_altitudes(double DELTA, double LAT, double H, double SF);
hour_angle_list* calculate_hour_angle(double DELTA, double LAT, sun_altitude_list* sa);
prayer_times_list* calculate_prayer_times(double TT, hour_angle_list *ha);

char* double_to_time(double time);

prayer_times_list* get_prayer_times_list(struct tm *time, int TimeZone, double LONG, double elevation, double LAT, int SF);
