#include "main.h"

#include "lemlib/chassis/chassis.hpp"
#include "util.hpp"

void arcade() {
		chassis.arcade(
			controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y), // forward/backward
			controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X) // turn
		);
	}