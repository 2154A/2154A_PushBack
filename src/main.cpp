#include <cstdarg>
#include "main.h"
#include "consts.hpp"
#include "lemlib/chassis/chassis.hpp"
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/abstract_motor.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
#include "util.hpp"
#include "autons.hpp"
#include "pros/motor_group.hpp"
#include "pros/rotation.hpp"
#include "pros/rtos.hpp"   
#include "pros/imu.hpp"
#include "intake.hpp"
#include "aaravselector.hpp"

//hi
/*______________*/




pros::Controller controller(pros::E_CONTROLLER_MASTER);


pros::Imu imu(IMU_PORT);   

bool in_driver_control = false;

// ---


/**
* MOTOR GROUPS:
*/
// front -> back  : ALL Normal
pros::MotorGroup left_motors({
   LEFT_FRONT_PORT
   , LEFT_MID_PORT
   , LEFT_BACK_PORT
}, pros::MotorGearset::blue);


// front -> back : ALL Reversed
pros::MotorGroup right_motors({
   RIGHT_FRONT_PORT
   , RIGHT_MID_PORT
   , RIGHT_BACK_PORT
}, pros::MotorGearset::blue);


pros::Rotation horizontal(HORIZ_TRACK_WHEEL_PORT);
pros::Rotation vertical(VERTICAL_TRACK_WHEEL_PORT);


/** liblem */


// 2.75 600 -> 450 3.25
lemlib::Drivetrain drivetrain(
   &left_motors, &right_motors,
   11.75,
   lemlib::Omniwheel::NEW_325,
   450,
   2
);


// lateral PID controller
lemlib::ControllerSettings lateral_controller(DT_LATERAL_P, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             DT_LATERAL_D, // derivative gain (kD)
                                             3, // anti windup
                                             1, // small error range, in inches
                                             100, // small error range timeout, in milliseconds
                                             3, // large error range, in inches
                                             500, // large error range timeout, in milliseconds
                                             0 // maximum acceleration (slew)
);


// angular PID controller
lemlib::ControllerSettings angular_controller(DT_ANGULAR_P, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             DT_ANGULAR_D, // derivative gain (kD)
                                             3, // anti windup
                                             1, // small error range, in inches
                                             100, // small error range timeout, in milliseconds
                                             3, // large error range, in inches
                                             500, // large error range timeout, in milliseconds
                                             0 // maximum acceleration (slew)
);




lemlib::TrackingWheel vertical_track(&vertical, lemlib::Omniwheel::NEW_275, -2.1);
lemlib::TrackingWheel horizontal_track(&horizontal, lemlib::Omniwheel::NEW_275,3);


lemlib::OdomSensors sensors(
                           &vertical_track, // vert nullptr test


                           nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                          
                           // &horizontal_track, // horizontal tracking wheel 1
                           &horizontal_track,
                          
                           nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                          
                           &imu // inertial sensor
);


lemlib::Chassis chassis(drivetrain, // drivetrain settings
                       lateral_controller, // lateral PID settings
                       angular_controller, // angular PID settings
                       sensors // odometry sensors
);




void initialize() {
   chassis.calibrate();
   show_loading_screen();
   drawSelector();
   pros::delay(700); // Give time for the screen to update
   pros::Task debugTask(updateDebugPanelTask);

}


void disabled() {
   chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);

}




void competition_initialize() {


}


void autonomous() {
   chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);
   test_auton();



}


void opcontrol() {
   in_driver_control = true;
   chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);


   while (true) {


      /**
        * DRIVING:
        */


      arcade();

      /**
       INTAKE: */

      if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
         run_intake_forward();
      } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
         run_intake_reverse();
      } else if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) {
         mid_goal_score();   
      }  else {
         intake_brake();
      }

   
   
       // delay to save system resources
      pros::delay(DRIVER_TICK);


   }
}