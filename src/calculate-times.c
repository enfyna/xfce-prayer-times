#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "calculate-times.h"

#ifdef RELEASE
#define RELEASE_CALCULATOR
#endif

#ifndef RELEASE_CALCULATOR
int main() {
  const double elevation = 800;
  const double LONG = 29.0605;
  const double LAT = 37.7737;
  const double sf = 1;
  const double Y = 2024;
  const double M = 9;
  double D = 1;
  const double H = 11;
  const double m = 0;
  const double s = 0;
  const double Z = 3;

  for (; D < 31; D++) {
    double jd = calculate_julian_days(Y, M, D, H, m, s, Z);
    double delta = calculate_sun_declination(jd);
    double et = calculate_equation_of_time(jd);
    double tt = calculate_transit_time(LONG, et, Z);

    sun_altitude_list *sa = calculate_sun_altitudes(delta, LAT, elevation, sf);
    hour_angle_list *ha = calculate_hour_angle(delta, LAT, sa);
    prayer_times_list *pt = calculate_prayer_times(tt, ha);

    printf("%2.0f.%.0f.%.0f : ", D, M, Y);
    printf("%s : ", double_to_time(pt->FAJR));
    printf("%s : ", double_to_time(pt->SUNRISE));
    printf("%s : ", double_to_time(pt->ZUHR));
    printf("%s : ", double_to_time(pt->ASR));
    printf("%s : ", double_to_time(pt->MAGHRIB));
    printf("%s \n", double_to_time(pt->ISHA));
  }
  return 0;
}
#endif

double calculate_julian_days(int Y, int M, int D, int H, int m, int s, int Z) {
  if (M <= 2) {
    Y -= 1;
    M += 12;
  }
  int A = Y / 100;
  int B = 2 - A + (A / 4);
  double JD = 1720994.5 + (int)(365.25 * Y) + (int)(30.6001 * (M + 1)) + B + D +
              ((H * 3600 + m * 60 + s) / 86400.0) - (Z / 24.0);
  return JD;
}

double calculate_sun_declination(double jd) {
  double T = 2 * PI * (jd - J2000_EPOCH) / 365.25;
  double Delta = 0.37877 +
                 23.264 * sin((1 * 57.297 * T - 79.547) * DEG_TO_RAD) +
                 0.3812 * sin((2 * 57.297 * T - 82.682) * DEG_TO_RAD) +
                 0.17132 * sin((3 * 57.297 * T - 59.722) * DEG_TO_RAD);
  return Delta;
}

double calculate_equation_of_time(double jd) {
  double U = (jd - J2000_EPOCH) / 36525;
  double L0 = 280.46607 + 36000.7698 * U;
  L0 *= DEG_TO_RAD;
  double ET1000 = -(7146 - 62 * U) * cos(L0) - (29 + 5 * U) * cos(2 * L0) +
                  (320 - 4 * U) * cos(3 * L0) - (1789 + 237 * U) * sin(L0) +
                  (9934 - 14 * U) * sin(2 * L0) + (74 + 10 * U) * sin(3 * L0) -
                  (212) * sin(4 * L0);
  double ET = ET1000 / 1000;
  return ET;
}

double calculate_transit_time(double LONGTITUDE, double ET, double Z) {
  double TT = 12 + Z - (LONGTITUDE / 15) - (ET / 60);
  return TT;
}

sun_altitude_list *calculate_sun_altitudes(double DELTA, double LAT, double H,
                                           double SF) {
  sun_altitude_list *sa = malloc(sizeof(sun_altitude_list));

  double SA_FAJR = -(FAJR_ANGLE);
  double SA_SUNRISE = -0.8333 - (0.0347 * sqrt(H)); // H -> elevation

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

hour_angle_list *calculate_hour_angle(double DELTA, double LAT,
                                      sun_altitude_list *sa) {
  hour_angle_list *ha = malloc(sizeof(hour_angle_list));
  double CHA;
  LAT *= DEG_TO_RAD;
  DELTA *= DEG_TO_RAD;

  CHA = (sin(sa->ASR * DEG_TO_RAD) - sin(LAT) * sin(DELTA)) /
        (cos(LAT) * cos(DELTA));
  ha->ASR = acos(CHA) * RAD_TO_DEG;

  CHA = (sin(sa->FAJR * DEG_TO_RAD) - sin(LAT) * sin(DELTA)) /
        (cos(LAT) * cos(DELTA));
  ha->FAJR = acos(CHA) * RAD_TO_DEG;

  CHA = (sin(sa->ISHA * DEG_TO_RAD) - sin(LAT) * sin(DELTA)) /
        (cos(LAT) * cos(DELTA));
  ha->ISHA = acos(CHA) * RAD_TO_DEG;

  CHA = (sin(sa->SUNRISE * DEG_TO_RAD) - sin(LAT) * sin(DELTA)) /
        (cos(LAT) * cos(DELTA));
  ha->SUNRISE = acos(CHA) * RAD_TO_DEG;
  ha->MAGHRIB = ha->SUNRISE;

  return ha;
}

prayer_times_list *calculate_prayer_times(double TT, hour_angle_list *ha) {
  prayer_times_list *pt = malloc(sizeof(prayer_times_list));

  pt->FAJR = TT - ha->FAJR / 15;
  pt->SUNRISE = TT - ha->SUNRISE / 15;
  pt->ZUHR = TT + DESCEND_CORRECTION;
  pt->ASR = TT + ha->ASR / 15;
  pt->MAGHRIB = TT + ha->MAGHRIB / 15;
  pt->ISHA = TT + ha->ISHA / 15;

  return pt;
}

char *double_to_time(double time) {
  char *str = malloc(sizeof(char) * 10);

  int hours = (int)time;
  int minutes = (int)((time - hours) * 60);
  int seconds = (int)((time - hours - (minutes / 60.0)) * 3600);

  sprintf(str, "%02d:%02d:%02d", hours, minutes, seconds);

  return str;
}

prayer_times_list* get_prayer_times_list(struct tm *time, int TimeZone, double LONG, double elevation, double LAT, int SF){
    double jd = calculate_julian_days(
            time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec, TimeZone);
    double delta = calculate_sun_declination(jd);
    double et = calculate_equation_of_time(jd);
    double tt = calculate_transit_time(LONG, et, TimeZone);
    sun_altitude_list *sa = calculate_sun_altitudes(delta, LAT, elevation, SF);
    hour_angle_list *ha = calculate_hour_angle(delta, LAT, sa);
    prayer_times_list *pt = calculate_prayer_times(tt, ha);
    return pt;
}
