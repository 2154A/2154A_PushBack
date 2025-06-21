#pragma "once"
#include <cstdarg>
#include "main.h"
#include "consts.hpp"
#include "autons.hpp"


void update_debug_info();
void toggle_debug_cb(lv_event_t* e);
void recalibrate_imu_cb(lv_event_t* e);
void show_loading_screen();
void auton_button_cb(lv_event_t* event);
void drawSelector();
void autonSelectorTask(void*);
void updateDebugPanelTask(void*);
