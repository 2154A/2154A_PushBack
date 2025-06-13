#include <cstdarg>
#include "main.h"
#include "consts.hpp"
#include "lemlib/chassis/chassis.hpp" 
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/abstract_motor.hpp"
#include "pros/llemu.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
#include "util.hpp"
#include "autons.hpp"
#include "autonselector.hpp"
#include "pros/screen.hpp"
#include "pros/llemu.hpp"
#include "pros/motor_group.hpp"



	
#include "pros/rotation.hpp"
#include "pros/rtos.hpp"	
#include "pros/imu.hpp"




pros::Controller controller(pros::E_CONTROLLER_MASTER);

pros::Imu imu(IMU_PORT);	

bool in_driver_control = false;

int autonNumber = 0;
const char* autonNames[] = {"Red Left", "Red Right", "Blue Left"};

lv_obj_t* auton_buttons[3];


void update_auton_button_colors() {
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_bg_color(auton_buttons[i], autonNumber == i ? lv_color_hex(0x00FF00) : lv_color_hex(0x444444), 0);
    }
}

void auton_button_cb(lv_event_t* event) {
    for (int i = 0; i < 3; i++) {
        if (event->target == auton_buttons[i]) {
            autonNumber = i;
            break;
        }
    }

    // Highlight selected button
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_bg_color(auton_buttons[i], autonNumber == i ? lv_color_hex(0x00FF00) : lv_color_hex(0x444444), 0);
    }
}

void drawSelector() {
    lv_obj_t* scr = lv_scr_act();

    for (int i = 0; i < 3; i++) {
        auton_buttons[i] = lv_btn_create(scr);
        lv_obj_set_size(auton_buttons[i], 140, 60);
        lv_obj_align(auton_buttons[i], LV_ALIGN_CENTER, (i - 1) * 160, 0); // -160, 0, +160
        lv_obj_add_event_cb(auton_buttons[i], auton_button_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label = lv_label_create(auton_buttons[i]);
        lv_label_set_text(label, autonNames[i]);
        lv_obj_center(label);
    }

    // Set initial highlight
    update_auton_button_colors();
}

void autonSelectorTask(void*) {
    // No longer needed with LVGL event system, so leave empty or remove
}


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
                                              20 // maximum acceleration (slew)
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






/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	drawSelector();
	pros::delay(3000); // Give time for the screen to update
	chassis.calibrate();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
	chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);
	
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */

 // autonomous selector


// //global console	

void competition_initialize() {

}
/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {

	chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);

	switch (autonNumber) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
	}
}
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	in_driver_control = true;
	chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);
	
	//hiiiii

	while (true) {

	// Print rotation sensor values for debugging

			/**
		 * DRIVING:
		 */

		arcade();
		
	

		// printf("arm pos: %d | target: %d\n", arm.encoder.get_position(), arm.target);
		// printf("arm current: %d\n", arm.arm_motor.get_current_draw());
		// intake.hues_debug();

		// delay to save system resources
		pros::delay(DRIVER_TICK);
	}
}