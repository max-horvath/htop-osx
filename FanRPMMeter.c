/*
htop
(c) 2013 Morgan Blackthorne
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "FanRPMMeter.h"
#include "Meter.h"

#include <time.h>

#include "debug.h"
#include "smc.h"

int FanRPMMeter_attributes[] = {
   FAN_RPM
};

static void FanRPMMeter_setValues(Meter* this, char* buffer, int size) {
    float rpm[8];
    
    rpm[0] = SMCGetFanRPM("F0Ac");
    rpm[1] = SMCGetFanRPM("F0Mx");
    rpm[2] = SMCGetFanRPM("F1Ac");
    rpm[3] = SMCGetFanRPM("F1Mx");
    rpm[4] = SMCGetFanRPM("F2Ac");
    rpm[5] = SMCGetFanRPM("F2Mx");
    rpm[6] = SMCGetFanRPM("F3Ac");
    rpm[7] = SMCGetFanRPM("F3Mx");

    //printf("0 = %.0f, 2 = %.0f, 4 = %.0f\n", rpm[0], rpm[2], rpm[4]);
    //printf("1 = %.0f, 3 = %.0f, 5 = %.0f\n", rpm[1], rpm[3], rpm[5]);

    this->values[0] = rpm[0];
    this->values[1] = rpm[1];
    this->values[2] = rpm[2];
    this->values[3] = rpm[3];
    this->values[4] = rpm[4];
    this->values[5] = rpm[5];
    this->values[6] = rpm[6];
    this->values[7] = rpm[7];
}

static void FanRPMMeter_display(Object* cast, RichString* out) {
    Meter* this = (Meter*)cast;
    char buffer[20];
    double hot_threshold = 4000.0; /* RPM */
    double warm_threshold = 3000.0; /* RPM */ 
    double cold_threshold = 1000.0; /* RPM */
    double raw_rpm_1, raw_rpm_2, raw_rpm_3, raw_rpm_4;
    double max_rpm_1, max_rpm_2, max_rpm_3, max_rpm_4;
    
    RichString_init(out);
    
    raw_rpm_1 = this->values[0]; max_rpm_1 = this->values[1];
    raw_rpm_2 = this->values[2]; max_rpm_2 = this->values[3];
    raw_rpm_3 = this->values[4]; max_rpm_3 = this->values[5];
    raw_rpm_4 = this->values[6]; max_rpm_4 = this->values[7];
    
    if (raw_rpm_1 > 0) {
        snprintf(buffer, 20, "%.0f RPM", raw_rpm_1);
        if (raw_rpm_1 > hot_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_HOT], buffer);
        } else if (raw_rpm_1 > warm_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_WARM], buffer);
        } else {
          RichString_append(out, CRT_colors[FAN_RPM_NORMAL], buffer);
        }
    } else {
        snprintf(buffer, 20, "-.- RPM");
        RichString_append(out, CRT_colors[FAN_RPM_NORMAL], buffer);
    }

    if (raw_rpm_2 > 0) {
        snprintf(buffer, 20, ", %.0f RPM", raw_rpm_2);
        if (raw_rpm_2 > hot_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_HOT], buffer);
        } else if (raw_rpm_2 > warm_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_WARM], buffer);
        } else {
          RichString_append(out, CRT_colors[FAN_RPM_NORMAL], buffer);
        }
    }

    if (raw_rpm_3 > 0) {
        snprintf(buffer, 20, ", %.0f RPM", raw_rpm_3);
        if (raw_rpm_3 > hot_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_HOT], buffer);
        } else if (raw_rpm_3 > warm_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_WARM], buffer);
        } else {
          RichString_append(out, CRT_colors[FAN_RPM_NORMAL], buffer);
        }
    }

    if (raw_rpm_4 > 0) {
        snprintf(buffer, 20, ", %.0f RPM", raw_rpm_4);
        if (raw_rpm_4 > hot_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_HOT], buffer);
        } else if (raw_rpm_4 > warm_threshold) {
          RichString_append(out, CRT_colors[FAN_RPM_WARM], buffer);
        } else {
          RichString_append(out, CRT_colors[FAN_RPM_NORMAL], buffer);
        }
    }
}

MeterType FanRPMMeter = {
    .setValues = FanRPMMeter_setValues,
    .display = FanRPMMeter_display,
    .mode = TEXT_METERMODE,
    .total = 100.0,
    .items = 8,
    .attributes = FanRPMMeter_attributes,
    .name = "FanRPM",
    .uiName = "Fan RPM",
    .caption = "Fan RPM: ",
};
