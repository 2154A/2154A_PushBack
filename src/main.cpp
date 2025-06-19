#include <cstdarg>
#include "main.h"
#include "consts.hpp"
#include "lemlib/chassis/chassis.hpp"
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/abstract_motor.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
#include "pros/motors.hpp"
#include "util.hpp"
#include "autons.hpp"
#include "autonselector.hpp"
#include "pros/motor_group.hpp"
#include "pros/rotation.hpp"
#include "pros/rtos.hpp"   
#include "pros/imu.hpp"
#include "intake.hpp"

//hi
/*______________*/




pros::Controller controller(pros::E_CONTROLLER_MASTER);


pros::Imu imu(IMU_PORT);   

bool in_driver_control = false;


int autonNumber = 0;


//Autonmous names
const char* autonNames[] = {"Left", "Right", "Solo", "Elims Rightm", "Elims Left"};


//Differnet Objects
lv_obj_t* auton_heading;
lv_obj_t* debug_label;
lv_obj_t* debug_btn;
lv_obj_t* debug_panel;
lv_obj_t* auton_list;


void update_debug_info() {
   lemlib::Pose pose = chassis.getPose();
   double x = pose.x;
   double y = pose.y;
   double theta = pose.theta;


   double left_temp = left_motors.get_temperature();
   double right_temp = right_motors.get_temperature();
   double avg_temp = (left_temp + right_temp) / 2.0;


   char buf[100];
   snprintf(buf, sizeof(buf), "X: %.1f\nY: %.1f\nT: %.1f\nAvg Temp: %.1f°C", x, y, theta, avg_temp);
   lv_label_set_text(debug_label, buf);
}


void toggle_debug_cb(lv_event_t* e) {
   bool hidden = lv_obj_has_flag(debug_panel, LV_OBJ_FLAG_HIDDEN);
   if (hidden) {
       lv_obj_t* button = (lv_obj_t*)lv_event_get_target(e);
       update_debug_info();
   } else {
       lv_obj_add_flag(debug_panel, LV_OBJ_FLAG_HIDDEN);
   }
}


void show_loading_screen() {   
   lv_obj_t* scr = (lv_obj_t *)lv_screen_active();


   // Create background
   lv_obj_t* bg = lv_obj_create(scr);
   lv_obj_set_size(bg, 480, 240);
   lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
   lv_obj_add_flag(bg, LV_OBJ_FLAG_SCROLLABLE);


   // Title
   lv_obj_t* label = lv_label_create(bg);
   lv_label_set_text(label, "2154A :3");
   lv_obj_set_style_text_color(label, lv_color_white(), 0);
   lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
   lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);


   // Progress bar
   lv_obj_t* bar = lv_bar_create(bg);
   lv_obj_set_size(bar, 300, 20);
   lv_obj_align(bar, LV_ALIGN_CENTER, 0, 20);
   lv_bar_set_range(bar, 0, 100);


   // Animate progress
   for (int i = 0; i <= 100; i += 5) {
       lv_bar_set_value(bar, i, LV_ANIM_OFF);
       pros::delay(30);
   }


   // Short delay before clearing
   pros::delay(250);
   lv_obj_clean(scr); // clear screen before showing auton selector
}




void auton_button_cb(lv_event_t* event) {
   lv_obj_t* button = (lv_obj_t*)lv_event_get_target(event);
   const char* name = lv_list_get_button_text(auton_list, button);


   for (int i = 0; i < 5; i++) {
       if (strcmp(name, autonNames[i]) == 0) {
           autonNumber = i;
           break;
       }
   }


   lv_label_set_text_fmt(auton_heading, "Selected: %s", autonNames[autonNumber]);


   // Highlight the selected button
   uint32_t count = lv_obj_get_child_count(auton_list);
   for (uint32_t i = 0; i < count; i++) {
       lv_obj_t* child = lv_obj_get_child(auton_list, i);
       lv_obj_t* label = lv_obj_get_child(child, 1); // second child is label


       if (i == autonNumber) {
           lv_obj_set_style_bg_color(child, lv_color_hex(0x0000FF), 0);
           lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
           lv_obj_set_style_text_color(label, lv_color_white(), 0);
       } else {
           lv_obj_set_style_bg_color(child, lv_color_hex(0x222222), 0);
           lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
           lv_obj_set_style_text_color(label, lv_color_white(), 0);
       }
   }


}


// IMU recalibration button callback
void recalibrate_imu_cb(lv_event_t* e) {
   imu.reset();
}


void drawSelector() {
   lv_obj_t* scr = (lv_obj_t *)lv_screen_active();


   auton_list = lv_list_create(scr);
   lv_obj_set_size(auton_list, 200, 180);
   lv_obj_align(auton_list, LV_ALIGN_CENTER, 0, 0);


   auton_heading = lv_label_create(scr);
   lv_label_set_text_fmt(auton_heading, "Selected: %s", autonNames[autonNumber]);
   lv_obj_align(auton_heading, LV_ALIGN_TOP_MID, 0, 10);


   for (int i = 0; i < 3; i++) {
       lv_obj_t* btn = lv_list_add_button(auton_list, LV_SYMBOL_PLAY, autonNames[i]);
       lv_obj_add_event_cb(btn, auton_button_cb, LV_EVENT_CLICKED, NULL);
   }


   // Debug toggle button (console icon)
   debug_btn = lv_button_create(scr);
   lv_obj_set_size(debug_btn, 40, 40);
   lv_obj_align(debug_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
   lv_obj_add_event_cb(debug_btn, toggle_debug_cb, LV_EVENT_CLICKED, NULL);


   lv_obj_t* icon_label = lv_label_create(debug_btn);
   lv_label_set_text(icon_label, LV_SYMBOL_SETTINGS); // Can replace with another symbol if desired
   lv_obj_center(icon_label);


   // Debug panel (initially hidden)
   debug_panel = lv_obj_create(scr);
   lv_obj_set_size(debug_panel, 140, 100);
   lv_obj_align(debug_panel, LV_ALIGN_TOP_RIGHT, -10, 60);
   lv_obj_add_flag(debug_panel, LV_OBJ_FLAG_HIDDEN);
   lv_obj_set_style_bg_color(debug_panel, lv_color_hex(0x222222), 0);
   lv_obj_set_style_border_width(debug_panel, 2, 0);
   lv_obj_set_style_border_color(debug_panel, lv_color_hex(0xFFFFFF), 0);


   debug_label = lv_label_create(debug_panel);
   lv_label_set_text(debug_label, "Debug Info");
   lv_obj_set_style_text_color(debug_label, lv_color_white(), 0);
   lv_obj_align(debug_label, LV_ALIGN_TOP_LEFT, 5, 5);


   // IMU recalibration button
   lv_obj_t* imu_btn = lv_button_create(scr);
   lv_obj_set_size(imu_btn, 40, 40);
   lv_obj_align(imu_btn, LV_ALIGN_TOP_LEFT, 10, 10);
   lv_obj_add_event_cb(imu_btn, recalibrate_imu_cb, LV_EVENT_CLICKED, NULL);


   lv_obj_t* imu_label = lv_label_create(imu_btn);
   lv_label_set_text(imu_label, LV_SYMBOL_REFRESH); // You can change to "IMU" if you prefer
   lv_obj_center(imu_label);
}


void autonSelectorTask(void*) {
   // No longer needed with LVGL event system, so leave empty or remove
}


void updateDebugPanelTask(void*) {
   while (true) {
       if (!lv_obj_has_flag(debug_panel, LV_OBJ_FLAG_HIDDEN)) {
           update_debug_info();
       }
       pros::delay(500);
   }
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
                           nullptr, // vert nullptr test


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
   pros::delay(3000); // Give time for the screen to update
   pros::Task debugTask(updateDebugPanelTask);

}


void disabled() {
   chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);

}




void competition_initialize() {


}


void autonomous() {
   chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);




}


void opcontrol() {
   in_driver_control = true;
   chassis.setBrakeMode(pros::motor_brake_mode_e::E_MOTOR_BRAKE_COAST);


   while (true) {


      /**
        * DRIVING:
        */


      arcade();


       // delay to save system resources
      pros::delay(DRIVER_TICK);


      /**
       INTAKE: */


      if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
         run_intake_forward();
         } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            run_intake_reverse();
               } else {
                  stop_intake();
               }
   }
}

