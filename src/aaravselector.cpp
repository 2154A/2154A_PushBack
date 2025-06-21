#include <cstdarg>
#include "main.h"
#include "consts.hpp"
#include "lemlib/chassis/chassis.hpp"
#include "autons.hpp"
#include "pros/motor_group.hpp"
#include "pros/rtos.hpp"
#include "pros/imu.hpp"
#include "liblvgl/core/lv_obj.h"

bool showAutonInfo = false;

int autonNumber = 0;

// Autonomous names
const char* autonNames[] = {"Left", "Right", "Solo", "Elims Rightm", "Elims Left"};

// LVGL objects
lv_obj_t* auton_heading;
lv_obj_t* debug_label;
lv_obj_t* debug_btn;
lv_obj_t* debug_panel;
lv_obj_t* auton_list;

void drawSelector();

void show_selected_auton_screen() {
    lv_obj_clean(lv_screen_active()); // clear existing objects
    lv_obj_t* scr = lv_screen_active();

    lv_obj_t* box = lv_obj_create(scr);
    lv_obj_set_size(box, 400, 120);
    lv_obj_align(box, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_width(box, 4, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* label = lv_label_create(box);
    lv_label_set_text_fmt(label, "%s", autonNames[autonNumber]);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFD600), 0); //yellow
    lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);

    lv_obj_t* tip = lv_label_create(scr);
    lv_label_set_text(tip, "Tap anywhere to return");
    lv_obj_set_style_text_color(tip, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(tip, LV_ALIGN_BOTTOM_MID, 0, -10);

    // set screen click handler
    lv_obj_add_event_cb(scr, [](lv_event_t* e){
        lv_obj_clean(lv_screen_active());
        drawSelector();
    }, LV_EVENT_CLICKED, NULL);
}   

void auton_button_cb(lv_event_t* event) {
    lv_obj_t* button = (lv_obj_t*)lv_event_get_target(event);
    const char* name = lv_list_get_button_text(auton_list, button);

    for (int i = 0; i < (int)(sizeof(autonNames) / sizeof(autonNames[0])); i++) {
        if (strcmp(name, autonNames[i]) == 0) {
            autonNumber = i;
            break;
        }
    }
    show_selected_auton_screen();
}

void update_debug_info() {
    lemlib::Pose pose = chassis.getPose();
    double x = pose.x;
    double y = pose.y;
    double theta = pose.theta;

    double left_temp_f = left_motors.get_temperature() * 9.0 / 5.0 + 32.0;
    double right_temp_f = right_motors.get_temperature() * 9.0 / 5.0 + 32.0;
    double avg_temp_f = (left_temp_f + right_temp_f) / 2.0;

    lv_label_set_text_fmt(debug_label, "X: %.1f\nY: %.1f\nT: %.1f\nAvg Temp: %.1f°F", x, y, theta, avg_temp_f);
}

void toggle_debug_cb(lv_event_t* e) {
    bool hidden = lv_obj_has_flag(debug_panel, (lv_obj_flag_t)LV_OBJ_FLAG_HIDDEN);
    if (hidden) {
        debug_panel->flags = (lv_obj_flag_t)(debug_panel->flags & ~(LV_OBJ_FLAG_HIDDEN));
        update_debug_info();
    } else {
        lv_obj_add_flag(debug_panel, (lv_obj_flag_t)LV_OBJ_FLAG_HIDDEN);
    }
}

void recalibrate_imu_cb(lv_event_t* e) {
    imu.reset();
}

void show_loading_screen() {
    lv_obj_t* scr = (lv_obj_t*)lv_screen_active();

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

void drawSelector() {
    lv_obj_t* scr = (lv_obj_t*)lv_screen_active();

    auton_list = lv_list_create(scr);
    lv_obj_set_size(auton_list, 200, 180);
    lv_obj_align(auton_list, LV_ALIGN_CENTER, 0, 0);

    auton_heading = lv_label_create(scr);
    lv_label_set_text_fmt(auton_heading, "Selected: %s", autonNames[autonNumber]);
    lv_obj_align(auton_heading, LV_ALIGN_TOP_MID, 0, 10);

    for (int i = 0; i < (int)(sizeof(autonNames) / sizeof(autonNames[0])); i++) {
        lv_obj_t* btn = lv_list_add_button(auton_list, LV_SYMBOL_PLAY, autonNames[i]);
        lv_obj_add_event_cb(btn, auton_button_cb, LV_EVENT_SHORT_CLICKED, NULL);
    }

    // Debug toggle button (console icon)
    debug_btn = lv_button_create(scr);
    lv_obj_set_size(debug_btn, 40, 40);
    lv_obj_align(debug_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(debug_btn, toggle_debug_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* icon_label = lv_label_create(debug_btn);
    lv_label_set_text(icon_label, LV_SYMBOL_SETTINGS);
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
    lv_label_set_text(imu_label, LV_SYMBOL_REFRESH);
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
