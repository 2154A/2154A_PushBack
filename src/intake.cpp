#include <cstdint>
#include "intake.hpp"
#include "main.h"
#include "pros/rtos.hpp"
#include "pros/motors.hpp"
#include "consts.hpp"


pros::Motor intake_left(PRE_ROLLER_PORT);
pros::Motor intake_right(ROLLER_PORT);


void run_intake_forward() {
   intake_left.move(127);
   intake_right.move(127);
}


void stop_intake() {
   intake_left.brake();
   intake_right.brake();
}


void run_intake_reverse() {
   intake_left.move(-127);
   intake_right.move(-127);
}



