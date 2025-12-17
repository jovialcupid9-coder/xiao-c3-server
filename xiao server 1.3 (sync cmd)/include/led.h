#pragma once
#include "prototypes.h"

#define BoardRGB_swapColors // esp32-s3-fh4r2 swaps R and G values, happened on both boards

#define OFF 0, 0, 0
#define WHITE 255, 255, 255
#define RED 255, 0, 0
#define GREEN 0, 255, 0
#define BLUE 0, 0, 255
#define YELLOW 255, 255, 0
#define CYAN 0, 255, 255
#define MAGENTA 255, 0, 255
#define ORANGE 255, 165, 0
#define PURPLE 128, 0, 128
#define LIME 0, 128, 0
#define TEAL 0, 128, 128
#define NAVY 0, 0, 128
#define SKYBLUE 135, 206, 235
#define INDIGO 75, 0, 130
#define VIOLET 238, 130, 238
#define CORAL 255, 127, 80


namespace led {
  void write(unsigned char R, unsigned char G, unsigned char B, int brightness = 5);
}

