#pragma once
#include <frt/frt.hpp>
#include <string>

using namespace FRT;
using namespace FRT::unit_literals;

struct ScanRes {
    public:
        int black1, black2, white1, white2;
};

class Config {
    public:
        static const constexpr int scan_whitemin = 60;
        static const constexpr int scan_blackmax = 20;
        static const constexpr int scan_voidmax = 5;
        static const constexpr int line_blackmax = 15;
        static const constexpr int line_greymin = 30;
        static const constexpr int line_whitemin = 60;
        static const constexpr int line_optimal = 20;
        static const constexpr int line_redmin = 100;
        static const constexpr int line_redmax = 175;
        static const constexpr double axle = 19.2;
        static const constexpr double wheel_diameter = 6.24;
        static const constexpr double sensor_distance = 20.5;
        static const constexpr double P = 0.9;
        static const constexpr double D = 0.5;
        static const constexpr int green_max = 60;
};

TachoMotor left(OUTPUT_C, cm(Config::wheel_diameter));
TachoMotor right(OUTPUT_A, cm(Config::wheel_diameter));
ColorSensor left_sensor(INPUT_2);
ColorSensor right_sensor(INPUT_1);
GyroSensor gyro(INPUT_4);

double bdist;

double leftb = 0;
double rightb = 0;