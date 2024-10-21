#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "calculate-times.h"

static double calculate_julian_days(struct tm *date, int Timezone)
{
    int Y = date->tm_year + 1900;
    int M = date->tm_mon + 1;
    int D = date->tm_mday;
    int H = date->tm_hour;
    int m = date->tm_min;
    int s = date->tm_sec;

    if (M <= 2) {
        Y -= 1;
        M += 12;
    }
    int A = Y / 100.0;
    int B = 2 - A + (A / 4.0);
    double JD = 1720994.5 
        + (int)(365.25 * Y) 
        + (int)(30.6001 * (M + 1)) 
        + B + D 
        + ((H * 3600 + m * 60 + s) / 86400.0) 
        - (Timezone / 24.0)
    ;
    return JD;
}

static double calculate_sun_declination(double jd)
{
    double T = 2 * PI * (jd - J2000_EPOCH) / 365.25;
    double Delta = 0.37877 
        + 23.264  * sin((1 * 57.297 * T - 79.547) * DEG_TO_RAD) 
        + 0.3812  * sin((2 * 57.297 * T - 82.682) * DEG_TO_RAD) 
        + 0.17132 * sin((3 * 57.297 * T - 59.722) * DEG_TO_RAD)
    ;
    return Delta;
}

static double calculate_equation_of_time(double jd)
{
    double U = (jd - J2000_EPOCH) / 36525;

    double L0 = 280.46607 + 36000.7698 * U;
    L0 *= DEG_TO_RAD;

    double ET1000 = 
        - (7146 -  62 * U) * cos(    L0) 
        - (  29 +   5 * U) * cos(2 * L0) 
        + ( 320 -   4 * U) * cos(3 * L0) 
        - (1789 + 237 * U) * sin(    L0) 
        + (9934 -  14 * U) * sin(2 * L0) 
        + (  74 +  10 * U) * sin(3 * L0) 
        - ( 212          ) * sin(4 * L0);
    double ET = ET1000 / 1000;
    return ET;
}

static double calculate_transit_time(
        double LONGTITUDE, double ET, double timezone
    )
{
    double TT = 12 + timezone - (LONGTITUDE / 15) - (ET / 60);
    return TT;
}

static sun_altitude_list* calculate_sun_altitudes(
        double DELTA, double LAT, double elevation, 
        double SF, double FAJR_ANGLE, double ISHA_ANGLE
    )
{
    sun_altitude_list* sa = malloc(sizeof(sun_altitude_list));

    double SA_FAJR = -(FAJR_ANGLE);
    double SA_SUNRISE = -0.8333 - (0.0347 * sqrt(elevation));

    double abs_res = fabs(DELTA - LAT);
    double tan_res = tan(abs_res * DEG_TO_RAD);
    double add_shadow = tan_res + SF;
    double SA_ASR = atan2(1, add_shadow) * RAD_TO_DEG;

    double SA_MAGHRIB = SA_SUNRISE;
    double SA_ISHA = -(ISHA_ANGLE);
    sa->FAJR = SA_FAJR;
    sa->SUNRISE = SA_SUNRISE;
    sa->ASR = SA_ASR;
    sa->MAGHRIB = SA_MAGHRIB;
    sa->ISHA = SA_ISHA;

    return sa;
}

static hour_angle_list* calculate_hour_angle(
        double DELTA, double LAT, sun_altitude_list* sa
    )
{
    hour_angle_list* ha = malloc(sizeof(hour_angle_list));
    double CHA;
    LAT *= DEG_TO_RAD;
    DELTA *= DEG_TO_RAD;

    CHA = (sin(sa->ASR * DEG_TO_RAD) 
        - sin(LAT) * sin(DELTA)) / (cos(LAT) * cos(DELTA));
    ha->ASR = acos(CHA) * RAD_TO_DEG;

    CHA = (sin(sa->FAJR * DEG_TO_RAD) 
        - sin(LAT) * sin(DELTA)) / (cos(LAT) * cos(DELTA));
    ha->FAJR = acos(CHA) * RAD_TO_DEG;

    CHA = (sin(sa->ISHA * DEG_TO_RAD) 
        - sin(LAT) * sin(DELTA)) / (cos(LAT) * cos(DELTA));
    ha->ISHA = acos(CHA) * RAD_TO_DEG;

    CHA = (sin(sa->SUNRISE * DEG_TO_RAD) 
        - sin(LAT) * sin(DELTA)) / (cos(LAT) * cos(DELTA));
    ha->SUNRISE = acos(CHA) * RAD_TO_DEG;
    ha->MAGHRIB = ha->SUNRISE;

    return ha;
}

prayer_time* double_time_to_time(double time)
{
    prayer_time* pt = malloc(sizeof(prayer_time));

    int hours = (int)time;
    int minutes = (int)((time - hours) * 60);
    int seconds = (int)((time - hours - (minutes / 60.0)) * 3600);

    pt->HOUR = hours;
    pt->MINUTE = minutes;
    pt->SECOND = seconds;
    return pt;
}

prayer_times_list* calculate_prayer_times(
        double TT, hour_angle_list* ha
    )
{
    prayer_times_list* pt = malloc(sizeof(prayer_times_list));

    pt->FAJR = double_time_to_time(TT - ha->FAJR / 15);
    pt->SUNRISE = double_time_to_time(TT - ha->SUNRISE / 15);
    pt->ZUHR = double_time_to_time(TT + DESCEND_CORRECTION);
    pt->ASR = double_time_to_time(TT + ha->ASR / 15);
    pt->MAGHRIB = double_time_to_time(TT + ha->MAGHRIB / 15);
    pt->ISHA = double_time_to_time(TT + ha->ISHA / 15);

    return pt;
}

char* prayer_time_to_string(prayer_time* pt_time)
{
    char* str = malloc(sizeof(char) * 9);
    if (str == NULL) { return NULL; }
    sprintf(str, "%02d:%02d:%02d",
        pt_time->HOUR, pt_time->MINUTE, pt_time->SECOND
    );
    return str;
}

prayer_times_list* get_prayer_times_list(
        double LONG, double LAT, double elevation,
        int SF, double FAJR_ANGLE, double ISHA_ANGLE
    )
{
    time_t now = time(NULL);
    struct tm *date = localtime(&now);

    int TimeZone = date->tm_gmtoff / 3600;
    double jd = calculate_julian_days(date, TimeZone);
    double delta = calculate_sun_declination(jd);
    double et = calculate_equation_of_time(jd);
    double tt = calculate_transit_time(LONG, et, TimeZone);
    sun_altitude_list* sa = calculate_sun_altitudes(delta, LAT, elevation, SF, FAJR_ANGLE, ISHA_ANGLE);
    hour_angle_list* ha = calculate_hour_angle(delta, LAT, sa);
    prayer_times_list* pt = calculate_prayer_times(tt, ha);
    free(sa);
    free(ha);
    return pt;
}

prayer_time* get_next_prayer(prayer_times_list* pt_list)
{
    time_t now = time(NULL);
    struct tm date = *localtime(&now);

    if (pt_list->SUNRISE->HOUR > date.tm_hour) {
        return pt_list->SUNRISE;
    }
    if (pt_list->SUNRISE->HOUR == date.tm_hour) {
        if (pt_list->SUNRISE->MINUTE > date.tm_min) {
            return pt_list->SUNRISE;
        }
        if (pt_list->SUNRISE->MINUTE == date.tm_min) {
            if (pt_list->SUNRISE->SECOND >= date.tm_sec) {
                return pt_list->SUNRISE;
            }
        }
    }
    if (pt_list->ZUHR->HOUR > date.tm_hour) {
        return pt_list->ZUHR;
    }
    if (pt_list->ZUHR->HOUR == date.tm_hour) {
        if (pt_list->ZUHR->MINUTE > date.tm_min) {
            return pt_list->ZUHR;
        }
        if (pt_list->ZUHR->MINUTE == date.tm_min) {
            if (pt_list->ZUHR->SECOND >= date.tm_sec) {
                return pt_list->ZUHR;
            }
        }
    }
    if (pt_list->ASR->HOUR > date.tm_hour) {
        return pt_list->ASR;
    }
    if (pt_list->ASR->HOUR == date.tm_hour) {
        if (pt_list->ASR->MINUTE > date.tm_min) {
            return pt_list->ASR;
        }
        if (pt_list->ASR->MINUTE == date.tm_min) {
            if (pt_list->ASR->SECOND >= date.tm_sec) {
                return pt_list->ASR;
            }
        }
    }
    if (pt_list->MAGHRIB->HOUR > date.tm_hour) {
        return pt_list->MAGHRIB;
    }
    if (pt_list->MAGHRIB->HOUR == date.tm_hour) {
        if (pt_list->MAGHRIB->MINUTE > date.tm_min) {
            return pt_list->MAGHRIB;
        }
        if (pt_list->MAGHRIB->MINUTE == date.tm_min) {
            if (pt_list->MAGHRIB->SECOND >= date.tm_sec) {
                return pt_list->MAGHRIB;
            }
        }
    }
    if (pt_list->ISHA->HOUR > date.tm_hour) {
        return pt_list->ISHA;
    }
    if (pt_list->ISHA->HOUR == date.tm_hour) {
        if (pt_list->ISHA->MINUTE > date.tm_min) {
            return pt_list->ISHA;
        }
        if (pt_list->ISHA->MINUTE == date.tm_min) {
            if (pt_list->ISHA->SECOND >= date.tm_sec) {
                return pt_list->ISHA;
            }
        }
    }
    prayer_time *next_prayer = malloc(sizeof(prayer_time));
    next_prayer->HOUR = pt_list->SUNRISE->HOUR + 24;
    next_prayer->MINUTE = pt_list->SUNRISE->MINUTE;
    next_prayer->SECOND = pt_list->SUNRISE->SECOND;
    return next_prayer;
}
