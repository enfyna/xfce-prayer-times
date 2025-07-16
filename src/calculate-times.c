#include <math.h>
#include <stdio.h>
#include <time.h>

#include "calculate-times.h"

double calc_julian_days(struct tm* date, int tz)
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
    return 1720994.5
        + (int)(365.25 * Y)
        + (int)(30.6001 * (M + 1))
        + B + D
        + ((H * 3600 + m * 60 + s) / 86400.0)
        - (tz / 24.0);
}

double calc_sun_declination(double jd)
{
    double T = 2 * PI * (jd - J2000_EPOCH) / 365.25;
    return 0.37877
        + 23.264 * sin((1 * 57.297 * T - 79.547) * DEG_TO_RAD)
        + 0.3812 * sin((2 * 57.297 * T - 82.682) * DEG_TO_RAD)
        + 0.17132 * sin((3 * 57.297 * T - 59.722) * DEG_TO_RAD);
}

double calc_equation_of_time(double jd)
{
    double U = (jd - J2000_EPOCH) / 36525;

    double L0 = 280.46607 + 36000.7698 * U;
    L0 *= DEG_TO_RAD;

    double ET1000 = -(7146 - 62 * U) * cos(L0)
        - (29 + 5 * U) * cos(2 * L0)
        + (320 - 4 * U) * cos(3 * L0)
        - (1789 + 237 * U) * sin(L0)
        + (9934 - 14 * U) * sin(2 * L0)
        + (74 + 10 * U) * sin(3 * L0)
        - (212) * sin(4 * L0);

    return ET1000 / 1000.0;
}

double calc_transit_time(double lon, double et, double tz)
{
    return 12 + tz - (lon / 15) - (et / 60);
}

calc_list calc_sun_altitudes(double delta, pt_args* args)
{
    calc_list sa = { 0 };

    double SA_FAJR = -(fajr_angle(args));
    double SA_SUNRISE = -0.8333 - (0.0347 * sqrt(elevation(args)));

    double abs_res = fabs(delta - latitude(args));
    double tan_res = tan(abs_res * DEG_TO_RAD);
    double add_shadow = tan_res + shadow_factor(args);
    double SA_ASR = atan2(1, add_shadow) * RAD_TO_DEG;

    double SA_MAGHRIB = SA_SUNRISE;
    double SA_ISHA = -(isha_angle(args));

    fajr(sa) = SA_FAJR;
    sunrise(sa) = SA_SUNRISE;
    asr(sa) = SA_ASR;
    maghrib(sa) = SA_MAGHRIB;
    isha(sa) = SA_ISHA;

    return sa;
}

calc_list calc_hour_angles(double delta, double lat, calc_list* sa)
{
    calc_list ha = { 0 };
    lat *= DEG_TO_RAD;
    delta *= DEG_TO_RAD;

    for (int i = 0; i < PT_TIME_COUNT; i++) {
        if (i == MAGHRIB) {
            continue;
        }
        double CHA = (sin(sa->items[i] * DEG_TO_RAD)
                         - sin(lat) * sin(delta))
            / (cos(lat) * cos(delta));
        // Make acos not return nan
        if (CHA >= 1) { CHA = 1; fprintf(stderr, "[ERROR] High CHA value!\n"); } 
        else if (CHA <= -1) { CHA = -1; fprintf(stderr, "[ERROR] Low CHA value!\n"); } 
        ha.items[i] = acos(CHA) * RAD_TO_DEG;
    }
    maghrib(ha) = sunrise(ha);

    return ha;
}

pt_time pt_double_to_time(double time)
{
    int hours = (int)time;
    int minutes = (int)((time - hours) * 60);
    int seconds = (int)((time - hours - (minutes / 60.0)) * 3600);

    return (pt_time) {
        .HOUR = hours,
        .MINUTE = minutes,
        .SECOND = seconds
    };
}

pt_list calc_pt_list(double tt, calc_list* ha, double descend_correction)
{
    pt_list pt;

    for (int i = 0; i < PT_TIME_COUNT; i++) {
        if (i == ZUHR) {
            continue;
        }
        double h = ha->items[i] / 15.0;
        if (i < ZUHR) {
            h *= -1;
        }
        pt.items[i] = pt_double_to_time(tt + h);
    }

    zuhr(pt) = pt_double_to_time(tt + descend_correction / 60.0);

    return pt;
}

pt_time_cstr pt_to_string(pt_time pt_time, int show_seconds)
{
    pt_time_cstr str;
    if (show_seconds) {
        snprintf(str.data, 9, "%02d:%02d:%02d",
            pt_time.HOUR, pt_time.MINUTE, pt_time.SECOND);
    } else {
        snprintf(str.data, 9, "%02d:%02d",
            pt_time.HOUR, pt_time.MINUTE);
    }
    return str;
}

pt_list pt_get_list(pt_args* args)
{
    time_t now = time(NULL);
    struct tm* date = localtime(&now);

    int tz = date->tm_gmtoff / 3600;
    double jd = calc_julian_days(date, tz);
    double delta = calc_sun_declination(jd);
    double et = calc_equation_of_time(jd);
    double tt = calc_transit_time(longitude(args), et, tz);
    calc_list sa = calc_sun_altitudes(delta, args);
    calc_list ha = calc_hour_angles(delta, latitude(args), &sa);
    return calc_pt_list(tt, &ha, descend_correction(args));
}

pt_time* pt_next_prayer(pt_list* pt_list)
{
    time_t now = time(NULL);
    struct tm date = *localtime(&now);

    for (int i = SUNRISE; i < PT_TIME_COUNT; i++) {
        if (pt_list->items[i].HOUR > date.tm_hour) {
            return &pt_list->items[i];
        }
        if (pt_list->items[i].HOUR == date.tm_hour) {
            if (pt_list->items[i].MINUTE > date.tm_min) {
                return &pt_list->items[i];
            }
            if (pt_list->items[i].MINUTE == date.tm_min) {
                if (pt_list->items[i].SECOND >= date.tm_sec) {
                    return &pt_list->items[i];
                }
            }
        }
    }

    return &pt_list->items[FAJR];
}
