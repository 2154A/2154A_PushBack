#include "intake.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/motor_group.hpp"
#include "pros/adi.hpp"
#include "pros/motors.hpp"
#include "consts.hpp"
#include "pros/motors.hpp"


pros::Motor pre_roller(PRE_ROLLER_PORT);
pros::Motor intake_static(ROLLER_PORT);
pros::Motor mid_goal(MID_GOAL_PORT);
pros::ADIDigitalOut counter_roller(COUNTER_ROLLER);
pros::ADIDigitalOut redirect(REDIRECT_ADI);
pros::ADIDigitalOut hood(HOOD);


pros::MotorGroup intake_motors({
   PRE_ROLLER_PORT,
   ROLLER_PORT,
   MID_GOAL_PORT
}, pros::MotorGearset::blue);

pros::MotorGroup mid_goal_cycle({
   PRE_ROLLER_PORT,
   ROLLER_PORT,
   -MID_GOAL_PORT
}, pros::MotorGearset::blue);

void run_intake_forward() {
   intake_motors.move(127);
   hood.set_value(false);
   redirect.set_value(false);
   counter_roller.set_value(false);
}

void top_goal_score() {
   mid_goal_cycle.move_velocity(127);
   redirect.set_value(false);
   hood.set_value(false);
   counter_roller.set_value(false);
}

void matchload(){
   intake_motors.move_velocity(127);
   redirect.set_value(false);
   hood.set_value(true);
   counter_roller.set_value(true);
}

void run_intake_reverse() {
   intake_motors.move(-127);
}

void intake_brake() {
   intake_motors.brake();
   hood.set_value(true);
   redirect.set_value(false);
   counter_roller.set_value(false);
}


void mid_goal_score(){
   mid_goal_cycle.move_velocity(127);
   redirect.set_value(true);
   hood.set_value(true);
   counter_roller.set_value(false);
}