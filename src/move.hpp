#pragma once
#include <frt/frt.hpp>
#include <iostream>
#include <cmath>
#include <functional>
#include <cstdlib>
#include "pd.hpp"
#include "other.hpp"

#define dobule double

using namespace FRT;
using namespace FRT::unit_literals;

void tacho_calibrate() {
    left.set_position(0cm);
    right.set_position(0cm);
    rightb = 0;
    leftb = 0;
}
double speed_factor(double dist, double pos, double acc_dist = 30, double max_speed = 68) {
    double acc_segment;
    double ms = max_speed - 10;
    acc_segment = ms / acc_dist;
    if (pos < dist / 2) {
        if (pos > acc_dist) {
            return ms + 10;
        }else {
            return pos * acc_segment + 10;
        }
    }else {
        if (pos < dist - acc_dist) {
            return ms + 10;
        }else {
            return (acc_dist - (pos - (dist - acc_dist))) * acc_segment + 10;
        }
    }
}


double const_sf(double dist, double pos, double start = 10, double end = 0, double acc_dist = 30, double max_speed = 68)
{
    double a = max_speed * max_speed / acc_dist;
    if (pos + (start * start / a) - end * end / a < dist / 2) {
        if (pos + start * start / a - end * end / a > acc_dist) {
            return max_speed;
        } else {
            return sqrt((pos + start * start / a) * a);
        }
    } else {
        if (pos + start * start / a - end * end / a < dist - acc_dist) {
            return max_speed;
        } else {
            return sqrt(abs((acc_dist - (pos - start * start / a - end  * end / a - (dist - acc_dist))) * a));
        }
    }
}


void tacho(
    Length auto distance, double start = 5, double end = 0, double acc_dist = 30, double max_speed = 60, std::function<void(double, double)> func = [](double i, double ii) {}, bool lgbreak = false, bool rbreak = false, bool lbreak = false)
{
    double dist = length_cast<cm>(distance).value;
    double lb = left.get_position().value;
    double rb = right.get_position().value;
    double correction = 0;
    double pos;
    double rbb = rightb;
    double lbb = leftb;
    leftb += dist;
    rightb += dist;
    while (1) {
        pos = (left.get_position().value - lb + right.get_position().value - rb) / 2;
        double sf = const_sf(abs(dist), abs(pos), start, end, acc_dist, max_speed);
        func(abs(pos), abs(dist));
        double corr = (leftb - left.get_position().value) - (rightb - right.get_position().value);
        Logger::debug(corr, left.get_position(), right.get_position(), left.get_speed_setpoint(), right.get_speed_setpoint());
        if (lgbreak && left_sensor.get_green() < Config::green_max) {
            tacho_calibrate();
            break;
        }
        if (lbreak && left_sensor.get_reflected_light_intensity() < Config::line_blackmax) break;
        if (rbreak && right_sensor.get_reflected_light_intensity() < Config::line_blackmax) break;
        if (dist > 0)
        {
            left.on(cm(sf + corr));
            right.on(cm(sf - corr));
            if (left.get_position().value >= leftb) break;
        } else {
            left.on(cm(-1 * sf + corr));
            right.on(cm(-1 * sf - corr));
            if (left.get_position().value <= leftb) break;
        }
    }
    if (end) return;
    left.stop();
    right.stop();
}

void move_on_cycle (Angle auto angle_, Length auto radius_, int direction = 1, double speed = 10) {
    double angle = angle_cast<deg>(angle_).value;
    double radius = length_cast<cm>(radius_).value;
    double dist = 4 * radius * pi / 360 * angle;
    double rfull = radius * 2 * pi;
    double lfull = (radius - Config::axle) * 2 * pi;

    double rdist = dist;
    double ldist = lfull / (rfull / rdist);

    double rb = right.get_position().value;
    double lb = left.get_position().value;

    double corr = 0;
    Logger::info(rfull, lfull, rdist, ldist);


    if (direction == -1) {
        leftb += rdist;
        rightb += ldist;
        lb = right.get_position().value;
        rb = left.get_position().value;
    } else {
        leftb += ldist;
        rightb += rdist;
    }
    while (1) {
        if (direction == 1) {
            if (dist > 0) {
                left.on(cm(speed / (lfull / rfull)));
                right.on(speed * 1cm);
                if (left.get_position().value >= leftb) break;
            } else {
                left.on(cm(speed / (lfull / rfull) * -1));
                right.on(speed * -1cm);
                if (left.get_position().value <= leftb) break;
            }
        } else {
            if (dist > 0) {
                right.on(cm(speed / (lfull / rfull)));
                left.on(speed * 1cm);
                if (right.get_position().value >= rightb) break;
            } else {
                right.on(cm(speed / (lfull / rfull) * -1));
                left.on(speed * -1cm);
                if (right.get_position().value <= rightb) break;
            }
        }
    }
    if (speed > 15) return;
    left.stop();
    right.stop();
}

void turn(Angle auto degrees) {
    int forward = 1;
    double degs = angle_cast<deg>(degrees).value;
    double dist = degs * (Config::axle / Config::wheel_diameter) / 360 * Config::wheel_diameter * pi;
    leftb -= dist;
    rightb += dist;
    double lb = left.get_position().value;
    double sf;
    double corr = 0;
    if (degs < 0) forward = -1;
    while (1) {
        Logger::debug(dist);
        sf = const_sf(abs(dist), abs(left.get_position().value - lb));
        corr = (leftb - left.get_position().value) - (rightb - right.get_position().value);
        left.on(cm(sf * -1 * forward));
        right.on(cm(sf * forward));
        if (forward == 1 && left.get_position().value <= leftb || forward == -1 && left.get_position().value >= leftb) break;
    }
    left.stop();
    right.stop();
}

void turnl(Angle auto degrees, bool cbreak = false)
{
    double degs = angle_cast<deg>(degrees).value;
    double one_deg = Config::axle * 2 / Config::wheel_diameter;
    double lb = left.get_position<deg>().value;
    leftb += one_deg * degs / 360 * Config::wheel_diameter * pi;
    int forward = 1;
    if (degrees < 0)
        forward = -1;
    right.set_stop_action(TachoMotor::stop_actions::hold);
    right.stop();
    Logger::debug(abs(one_deg * degs), one_deg, degs);
    while (abs(one_deg * degs) > abs(left.get_position<deg>().value - lb))
    {
        left.on(const_sf(abs(Config::wheel_diameter * pi / 360 * one_deg * degs), abs(left.get_position().value - Config::wheel_diameter * pi / 360 * lb), 10, 0, 30, 40) * forward * 1cm);
        Logger::debug(const_sf(abs(Config::wheel_diameter * pi / 360 * one_deg * degs), abs(left.get_position().value - Config::wheel_diameter * pi / 360 * lb), 10, 0, 30, 40));
        if (cbreak && left_sensor.get_reflected_light_intensity() < Config::line_blackmax) {
            //bdist = round((left.get_position<deg>().value - lb) / one_deg);
            tacho_calibrate();
            break;
        }
    }
    left.set_stop_action(TachoMotor::stop_actions::brake);
    left.stop();
    right.stop();
}
void turnr(Angle auto degrees, bool cbreak = false, double max = 40)
{
    double degs = angle_cast<_deg>(degrees).value;
    double one_deg = Config::axle * 2 / Config::wheel_diameter;
    double rb = right.get_position<deg>().value;
    rightb += one_deg * degs / 360 * Config::wheel_diameter * pi;
    int forward = 1;
    if (degrees < 0) forward = -1;
    left.set_stop_action(TachoMotor::stop_actions::hold);
    left.stop();
    Logger::debug(abs(one_deg * degs), right.get_position() - rb);
    while (abs(one_deg * degs) > abs(right.get_position<_deg>().value - rb))
    {
        right.on(const_sf(abs(Config::wheel_diameter * pi / 360 * one_deg * degs), abs(right.get_position().value - Config::wheel_diameter * pi / 360 * rb), 10, 0, 30, max) * forward * 1cm);
        Logger::debug(const_sf(abs(Config::wheel_diameter * pi / 360 * one_deg * degs), abs(right.get_position().value - Config::wheel_diameter * pi / 360 * rb), 10, 0, 30, max));
        if (cbreak && right_sensor.get_reflected_light_intensity() < Config::line_blackmax) {
            tacho_calibrate();
            break;
        }
    }
    left.set_stop_action(TachoMotor::stop_actions::brake);
    left.stop();
    right.stop();
}

void towall(int direction = 1, double sleepy = 3)
{
    left.on(15cm * direction);
    right.on(15cm * direction);
    for(int i = 0; i < sleepy * 10; i++){
    if(left.is_overloaded() && right.is_overloaded()) break;
    sleep(100ms);
    }
    right.stop();
    left.stop();
    tacho_calibrate();
}

void line_follow (
    Length auto distance, int sensor_in = 2, int side = 1, double start = 5, double end = 0, std::function<void(double, double)> func = [](double pos, double dist) {}, double acc_dist = 30, double max = 65, double optimal = 35, bool rbreak = false, bool lbreak = false) {

    ColorSensor sensor("ev3-ports:in" + std::to_string(sensor_in));
    ColorSensor marker(INPUT_4);
    double dist = length_cast<cm>(distance).value;
    double correction = 0;
    double lb = left.get_position().value;
    double rb = right.get_position().value;
    double pos = 0;
    PD pd(optimal, Config::P, Config::D);

    leftb += dist;
    rightb += dist;

    if (dist > 0)
    {
        while (left.get_position().value - lb < dist)
        {
            func(abs(left.get_position().value - lb), abs(dist));
            pos = abs(left.get_position().value - lb);

            if (rbreak && right_sensor.get_reflected_light_intensity() < 10)
                break;
            else if (lbreak && left_sensor.get_reflected_light_intensity() < 10)
                break;

            correction = pd.update(sensor.get_reflected_light_intensity());

            double speed = const_sf(abs(dist), pos, start, end, acc_dist, max);
            double l = speed - correction * side;
            double r = speed + correction * side;

            if(sensor.port == right_sensor.port) {
                double l = speed - correction / 5 * side;
                double r = speed + correction * side;
            }
            if(sensor.port == left_sensor.port) {
                double l = speed - correction * side;
                double r = speed + correction / 5 * side;
            }

            left.on(cm(l));
            right.on(cm(r));
        }
    } else {
        PD asd(optimal, 0.5, 0.25);
        while (-1 * pos > dist)
        {
            func(abs(pos), abs(dist));
            pos = abs(left.get_position().value - lb);

            if (rbreak && right_sensor.get_reflected_light_intensity() <= Config::line_blackmax)
                break;
            else if (lbreak && left_sensor.get_reflected_light_intensity() <= Config::line_blackmax)
                break;

            correction = asd.update(sensor.get_reflected_light_intensity());
            double speed = const_sf(abs(dist), pos, start, end, acc_dist, max);
            double l = speed - correction / 3 * side;
            double r = speed + correction * side;

             if(sensor.port == right_sensor.port) {
                double l = speed - correction / 5 * side;
                double r = speed + correction * side;
            }
            if(sensor.port == left_sensor.port) {
                double l = speed - correction * side;
                double r = speed + correction / 5 * side;
            }

            Logger::debug(correction, l, r, sensor.get_reflected_light_intensity(), pos);

            left.on(cm(l) * -1);
            right.on(cm(r) * -1);
        }
    }
    if (end) return;
    left.stop();
    right.stop();
}