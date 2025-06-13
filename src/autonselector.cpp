#include "autonselector.hpp"
#include "pros/llemu.hpp"
#include "pros/rtos.hpp"

void autonSelector() {
	pros::lcd::print(0, "Use L/R buttons to select auton");

	while (true) {
		if (pros::lcd::read_buttons() == LCD_BTN_LEFT) {
			autonNumber = (autonNumber - 1 + 3) % 3;
			pros::lcd::print(1, "Auton: %d", autonNumber);
			pros::delay(300);
		} else if (pros::lcd::read_buttons() == LCD_BTN_RIGHT) {
			autonNumber = (autonNumber + 1) % 3;
			pros::lcd::print(1, "Auton: %d", autonNumber);
			pros::delay(300);
		}
		pros::delay(20);
	}
}