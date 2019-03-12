#pragma once

#define INP_MAX_RAW_BYTES           64
#define INP_MAX_MOUSE_AXES          13
#define INP_MAX_TABLET_COORDS       7
#define INP_MAX_TABLET_AXES         6
#define INP_MAX_CONTROLLER_COORDS   13

#define INP_DEV_TYPE_UNKNOWN        0
#define INP_DEV_TYPE_OTHER          1
#define INP_DEV_TYPE_KEYBOARD       2
#define INP_DEV_TYPE_MOUSE          3
#define INP_DEV_TYPE_TABLET         4
#define INP_DEV_TYPE_CONTROLLER     5

#define INP_KBD_EVENT_FLAG_RELEASE  1

struct inpKeyboardEvent
{
    unsigned int Key;
    unsigned int Flags;
};

struct inpMouseEvent
{
    unsigned int ButtonsPressed;
    unsigned int ButtonsHeld;
    unsigned int ButtonsReleased;
    int Delta[INP_MAX_MOUSE_AXES];
};

struct inpTabletEvent
{
    unsigned int ButtonsPressed;
    unsigned int ButtonsHeld;
    unsigned int ButtonsReleased;
    int Coords[INP_MAX_TABLET_COORDS];
    int Delta[INP_MAX_TABLET_AXES];
};

struct inpControllerEvent
{
    unsigned int ButtonsPressed;
    unsigned int ButtonsHeld;
    unsigned int ButtonsReleased;
    int Coords[INP_MAX_CONTROLLER_COORDS];
};
