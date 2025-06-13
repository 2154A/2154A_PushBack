#ifndef GUARD_SCALED_IMU_H
#define GUARD_SCALED_IMU_H

#include "pros/imu.hpp"

namespace robot_systems {
namespace modified_api {
class ScaledIMU : public pros::IMU {
public:
    ScaledIMU(int port, double scalar)
        : pros::IMU(port), port_(port), scalar_(scalar) {}

    double get_rotation() const override {
        return pros::c::imu_get_rotation(port_) * scalar_;
    }

private:
    int port_;
    double scalar_;
};
} // namespace modifiedAPI
} // namespace robotSystems
#endif