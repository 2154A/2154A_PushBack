/**
 * IMPORTS:
*/

#include "consts.hpp"
#include "lemlib/chassis/chassis.hpp"
#include "util.hpp"

#include "main.h"
#include "arm.hpp"
#include "intake.hpp"
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/abstract_motor.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
	
#include "pros/rotation.hpp"
#include "pros/rtos.hpp"
#include <cstddef>

/**
 * GENERAL STUFF:
 */

// controller definition
pros::Controller controller(pros::E_CONTROLLER_MASTER);

pros::Imu imu(3);

bool in_driver_control = false;
bool color_sorting = true;

// ---

/**
 * MOTOR GROUPS:
 */
// front (mogo) -> back (intake) : ALL REVERSED
pros::MotorGroup left_motors({
	LEFT_FRONT_PORT
	, LEFT_MID_PORT
	, LEFT_BACK_PORT
}, pros::MotorGearset::blue);

// front (mogo) -> back (intake) : ALL NORMAL
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

// chnged to positive because tracking wheel is on mogo mech side
// which is on front of robot!

// // theoretical actual offset: (1.506000 + 0.864000) / 2 = 1.185
// // 1.625 -> 1.185
// lemlib::TrackingWheel horizontal_track(&horizontal, lemlib::Omniwheel::NEW_275, 1.185);
// ACTUAL measured offset of: vertical track is -11/16ths
// -3.5 -> -0.6875
lemlib::TrackingWheel vertical_track(&vertical, 2, -0.6875);
lemlib::TrackingWheel horizontal_track(&horizontal, 2.75,1.85 );

lemlib::OdomSensors sensors(
							nullptr, // vert nullptr test

							nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            
							// &horizontal_track, // horizontal tracking wheel 1
							nullptr,
                            
							nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            
							&imu // inertial sensor
);

lemlib::Chassis chassis(drivetrain, // drivetrain settings
                        lateral_controller, // lateral PID settings
                        angular_controller, // angular PID settings
                        sensors // odometry sensors
);

/**
 * SUBSYSTEM INITIALIZATION:
*/

Intake intake = Intake(
	// was not negative before
	{-INTAKE_PORT}
    , INTAKE_LIFT_PORT
	, pros::E_MOTOR_BRAKE_COAST	// brake mode of intake
	// , COLOR_PORT
	, INTAKE_COLOR_SENSOR_PORT
	, INTAKE_DIST_SENSOR_PORT
	, 1
);

MogoMech mogo = MogoMech(MOGO_MECH_PORT);

Arm arm = Arm(
	ARM_PORT
	, pros::E_MOTOR_BRAKE_HOLD
	, 11
	, &intake
);

Doinker doinker = Doinker(LEFT_DOINKER_PORT, RIGHT_DOINKER_PORT);

// RushMech rush_mech = RushMech(RUSH_MECH_PORT);

/**
 * VARIABLE DEFINITION:
 */

// bool LEFT_state = false;
bool L1_state = false;
bool B_state = false;

// pros::Task print_pose(print_robot_pos, &chassis);


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	chassis.calibrate();

	// auton_table.auton_populate(
    //     {
	// 		// red neg
	// 		Auton(
	// 			red_neg, Alliance::RED, Corner::NEGATIVE, "Red Neg Solo AWP", "Scores 4 rings", 4, 0, false
	// 		)
			
	// 		// red pos
	// 		// Auton(red_positive_awp_nostack, Alliance::RED, Corner::POSITIVE, "Red Swap", "No ring stack! + 3 rings", 2, 0, true),
	// 		, Auton(
	// 			red_rush, Alliance::RED, Corner::POSITIVE, "Red Rush (2 ring)", "Intercepts opponent auton; scores 2 rings", 2, 0, false
	// 		)
	// 		// Auton(red_negative_five_ring, RED, NEGATIVE, "Red 5 ring", "5 rings on mogo, WITH RING STACK", 5, 0, true),
			
	// 		// blue neg
	// 		, Auton(
	// 			blue_negative_awp
	// 			, Alliance::BLUE
	// 			, Corner::NEGATIVE
	// 			, "Blue Neg"
	// 			, "Aarav's Function"
	// 			, 2
	// 			, 0
	// 			, false
	// 		)
			
			
	// 		// blue pos

	// 	}
	// );
		
		
	screen_init();
	
	
	// pros::Task distance_calc(updateLoop);

	// set up color sort and auto-arm alg
	pros::Task color_sort(&update_sort, &intake);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {

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
void competition_initialize() {};

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
	
		chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);
	
//red neg
	// red_neg_wp();
	// red_negative_elims();

//red pos
	// red_rush();
	// red_pos_wp();
	// red_pos_elims();

//blue neg
	fun_auto();
	// blue_negative_wp();
	// blue_negative_elims();

//blue pos
	// blue_rush();
	// blue_pos_wp();
	// 	

	
	
	// red_sawp();
};

/**
 * Runs the operator control code. This function will be started in its *own task*
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * *If no competition control is connected (the program is run straight from the controller),
 * this function will run immediately following initialize()*
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	// pros::Task poseTask(opcontrolTask, &chassis);

	in_driver_control = true;

    pros::Task disableColorSort(disable_color_sort, &color_sorting);
    pros::Task unclampMogo(unclamp_mogo, &mogo);

	// brake mode back to coast!
	chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);
	intake.intake_motors.set_brake_mode_all(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);

	arm.set_pos(arm.INIT_POS);

	// starting hue tuned based on whatever venue we're at

	pros::Task arm_task(&arm_update, &arm);

	while (true) {
		/**
		* CONTROL FETCHING:
		*/
		
		///// HOLD controls

		// intake
		bool R1_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
		// outtake
		bool R2_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);

		// force pos
		bool B_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_B);

		// ---

		///// TOGGLE controls
		// mogo mech
		bool L2_new_press = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2);
		// doinker
		bool Y_new_press = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y);
		// toggle color sort
        bool A_new_press = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A);
		// arm -- dunk pos
		bool LEFT_new_press = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT);
        // arm -- loadin
		bool DOWN_new_press = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN);
		// arm -- start pos
        bool RIGHT_new_press = controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT);
		// arm -- cycle score 
		bool L1_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
        
		/**
		 * ARM:
		 */

		// FORCE POS
		if (B_pressed) {
			if (B_state == false) {
				B_state = true;
				arm.force();
			}
		// only want to jump in here if both are true,
		// bc of the else if that only allows one branch to run
		}
		else if (!B_pressed && B_state == true) {
			B_state = false;
			// falling edge; JUST released
			arm.release();
		}
		
		// SCORE POS
		else if (L1_pressed) {
			if (L1_state == false) {
				L1_state = true;
				arm.score();
			}
		} else if (!L1_pressed && L1_state == true) {
			L1_state = false;
			arm.release();
		}
		
		else if (DOWN_new_press) {
			arm.set_pos(arm.LOADIN_POS);
		} else if (RIGHT_new_press) {
			arm.set_pos(arm.START_POS);
		}
		else if (LEFT_new_press) {
			arm.set_pos(arm.DUNK_POS);
		}

        // /**
        //  * TOGGLE COLOR SORT:
        //  */
        // if (A_new_press) {
        //     printf("A new press!\n");
        //     // Enabling.
        //     if (color_sorting == false) {
        //         // printf("ENABLING color sort\n");
        //         color_sorting = true;
        //         controller.rumble("--");
        //     }
        //     // Disabling.
        //     else if (color_sorting == true) {
        //         // printf("DISABLING color sort\n");
        //         color_sorting = false;
        //         controller.rumble("...");
        //     }
        // }
        // // controller.clear_line(0);
        // show color sort status
        controller.set_text(0, 0, color_sorting ? "ENABLED  " : "DISABLED  ");
        // // in case controller sucks
        // // printf("Color Sort %s\n", color_sorting ? "ENABLED" : "DISABLED");

		/**
		 * DOINKER:
	     */
		if (Y_new_press) {
			doinker.toggle();
		}
	
		/**
		 * INTAKE:
		 */
		// handled by a task!

		/**
		 * MOGO:
		 */

		if (L2_new_press) {
			mogo.toggle();
		}

		// if (LEFT_new_press){
		// 	intake.outtake();
		// 	pros::delay(20);
		// 	intake.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
		// 	arm.set_pos(arm.DUNK_POS);
		// }

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