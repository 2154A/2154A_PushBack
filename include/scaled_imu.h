#ifndef GUARD_SCALED_IMU_H
#define GUARD_SCALED_IMU_H

#include "pros/imu.hpp"

namespace robot_systems {
namespace modified_api {
class ScaledIMU : public pros::IMU {
    public:
        ScaledIMU(int port, double scalar)
            : pros::IMU(port),
            m_port(port),
            m_scalar(scalar) {}

        virtual double get_rotation() const { return pros::c::imu_get_rotation(m_port) * m_scalar; }
    private:
        const int m_port;
        const double m_scalar;
};
} // namespace modifiedAPI
} // namespace robotSystems
#endif