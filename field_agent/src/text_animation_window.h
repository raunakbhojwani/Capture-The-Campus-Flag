#pragma once

#include <pebble.h>

#define TEXT_ANIMATION_WINDOW_DURATION 1000   // Duration of each half of the animation
#define TEXT_ANIMATION_WINDOW_DISTANCE 5    // Pixels the animating text move by
#define TEXT_ANIMATION_WINDOW_INTERVAL 1000 // Interval between timers

void text_animation_window_push(char* text_1, char* text_2);