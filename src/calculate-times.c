#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "calculate-times.h"

double calc_julian_days(struct tm *date, int tz)
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
        - (tz / 24.0)
    ;
    return JD;
}

double calc_sun_declination(double jd)
{
    double T = 2 * PI * (jd - J2000_EPOCH) / 365.25;
    double Delta = 0.37877 
        + 23.264  * sin((1 * 57.297 * T - 79.547) * DEG_TO_RAD) 
        + 0.3812  * sin((2 * 57.297 * T - 82.682) * DEG_TO_RAD) 
        + 0.17132 * sin((3 * 57.297 * T - 59.722) * DEG_TO_RAD)
    ;
    return Delta;
}

double calc_equation_of_time(double jd)
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

double calc_transit_time(double lon, double et, double tz)
{
    double TT = 12 + tz - (lon / 15) - (et / 60);
    return TT;
}

calc_list calc_sun_altitudes(double delta, pt_args* args)
{
    calc_list sa = {0};

    double SA_FAJR = -(args->fajr_angle);
    double SA_SUNRISE = -0.8333 - (0.0347 * sqrt(args->elevation));

    double abs_res = fabs(delta - args->latitude);
    double tan_res = tan(abs_res * DEG_TO_RAD);
    double add_shadow = tan_res + args->shadow_factor;
    double SA_ASR = atan2(1, add_shadow) * RAD_TO_DEG;

    double SA_MAGHRIB = SA_SUNRISE;
    double SA_ISHA = -(args->isha_angle);

    sa.items[FAJR] = SA_FAJR;
    sa.items[SUNRISE] = SA_SUNRISE;
    sa.items[ASR]= SA_ASR;
    sa.items[MAGHRIB] = SA_MAGHRIB;
    sa.items[ISHA] = SA_ISHA;

    return sa;
}

calc_list calc_hour_angles(double delta, double lat, calc_list* sa)
{
    calc_list ha = {0};
    lat *= DEG_TO_RAD;
    delta *= DEG_TO_RAD;

    for (int i = 0; i < PT_TIME_COUNT; i++) {
        if (i == MAGHRIB) {
            continue;
        }
        double CHA = (sin(sa->items[i] * DEG_TO_RAD) 
            - sin(lat) * sin(delta)) / (cos(lat) * cos(delta));
        ha.items[i] = acos(CHA) * RAD_TO_DEG;
    }
    ha.items[MAGHRIB] = ha.items[SUNRISE];

    return ha;
}

pt_time* pt_double_to_time(double time)
{
    pt_time* pt = malloc(sizeof(pt_time));

    int hours = (int)time;
    int minutes = (int)((time - hours) * 60);
    int seconds = (int)((time - hours - (minutes / 60.0)) * 3600);

    pt->HOUR = hours;
    pt->MINUTE = minutes;
    pt->SECOND = seconds;
    return pt;
}

pt_list* calc_pt_list(double tt, calc_list* ha)
{
    pt_list* pt = malloc(sizeof(pt_list));

    for (int i = 0; i < PT_TIME_COUNT; i++) {
        if (i == ZUHR) {
            continue;
        }
        double h = ha->items[i] / 15.0;
        if (i < ZUHR) {
            h *= -1;
        }
        pt->items[i] = pt_double_to_time(tt + h);
    }

    pt->items[ZUHR] = pt_double_to_time(tt + DESCEND_CORRECTION);

    return pt;
}

char* pt_to_string(pt_time* pt_time)
{
    char* str = malloc(sizeof(char) * 9);
    if (str == NULL) { return NULL; }
    sprintf(str, "%02d:%02d:%02d",
        pt_time->HOUR, pt_time->MINUTE, pt_time->SECOND
    );
    return str;
}

pt_list* pt_get_list(pt_args* args)
{
    time_t now = time(NULL);
    struct tm *date = localtime(&now);

    int tz = date->tm_gmtoff / 3600;
    double jd = calc_julian_days(date, tz);
    double delta = calc_sun_declination(jd);
    double et = calc_equation_of_time(jd);
    double tt = calc_transit_time(args->longitude, et, tz);
    calc_list sa = calc_sun_altitudes(delta, args);
    calc_list ha = calc_hour_angles(delta, args->latitude, &sa);
    pt_list* pt = calc_pt_list(tt, &ha);
    return pt;
}

pt_time* pt_next_prayer(pt_list* pt_list)
{
    time_t now = time(NULL);
    struct tm date = *localtime(&now);

    for (int i = SUNRISE; i < PT_TIME_COUNT; i++) {
        if (pt_list->items[i]->HOUR > date.tm_hour) {
            return pt_list->items[i];
        }
        if (pt_list->items[i]->HOUR == date.tm_hour) {
            if (pt_list->items[i]->MINUTE > date.tm_min) {
                return pt_list->items[i];
            }
            if (pt_list->items[i]->MINUTE == date.tm_min) {
                if (pt_list->items[i]->SECOND >= date.tm_sec) {
                    return pt_list->items[i];
                }
            }
        }
    }

    return pt_list->items[FAJR];
}
