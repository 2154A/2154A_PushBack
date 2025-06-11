#include "main.h"

#include "lemlib/chassis/chassis.hpp"
#include "lemlib/pose.hpp"
#include "pros/rtos.h"
#include "pros/rtos.hpp"
#include "util.hpp"
#include <numbers>


//test

void print_robot_pos(void *chassisVoid) {
  lemlib::Chassis *chassis = static_cast<lemlib::Chassis *>(chassisVoid);

  while (true) {
    lemlib::Pose pos = chassis->getPose();

    std::cout << "x: " << pos.x << " | y: " << pos.y
              << " | theta: " << pos.theta << std::endl;

    pros::delay(100);
  }
}
void moveDist(double x, int timeout, bool forwards = true, float maxSpeed = 127) {
  lemlib::Pose p = chassis.getPose();
  if (!forwards) {
    p.theta += 180;
  };
  chassis.moveToPoint(p.x + x * sin(p.theta), p.y + x * cos(p.theta), timeout,
                      {.forwards = forwards, .maxSpeed = maxSpeed});
}

/* Test Autons*/
void test_auton() {
  chassis.setPose(0, 0, 0);
  
  



}

void no_auton() { 
  chassis.setPose(0, 0, 0); 
}

void move_forward() {
  chassis.setPose(0, 0, 0);

  chassis.moveToPoint(0, 7, 1000);
  waitd;
  
}