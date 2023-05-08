/* Establish a certain threshold to move. */
#define LEFT_THRESHOLD  1300
#define RIGHT_THRESHOLD 2700
#define UP_THRESHOLD    1300
#define DOWN_THRESHOLD  2700
#define RESET          0

#define COMMAND_LEFT   1
#define COMMAND_RIGHT  2
#define COMMAND_UP     4
#define COMMAND_DOWN   8

#define JUMP_TIMEOUT 12
#define DUCK_TIMEOUT 4

#define BUTTON_PIN 25
int xyPins[] = {13, 12};