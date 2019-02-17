#pragma once

#include <types.hpp>

enum class VirtualKey : uint8_t
{
    Null = 0x00, LeftButton, RightButton, Cancel, MiddleButton, ExtraButton, ExtraButton2, Undefined07,
    Back, Tab, Undefined0A, Undefined0B, Clear, Return, Undefined0E, Undefined0F,

    Undefined10, Undefined11, Undefined12, Pause, Capital, Kana, Undefined16, Junja,
    Final, Kanji, Undefined1A, Escape, Convert, NonConvert, Accept, ModeChange,

    Space, Prior, Next, End, Home, Left, Up, Right,
    Down, Select, Print, Execute, Snapshot, Insert, Delete, Help,

    Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7,
    Key8, Key9, Undefined3A, Undefined3B, Undefined3C, Undefined3D, Undefined3E, Undefined3F,

    Undefined40, KeyA, KeyB, KeyC, KeyD, KeyE, KeyF, KeyG,
    KeyH, KeyI, KeyJ, KeyK, KeyL, KeyM, KeyN, KeyO,

    KeyP, KeyQ, KeyR, KeyS, KeyT, KeyU, KeyV, KeyW,
    KeyX, KeyY, KeyZ, LWin, RWin, Apps, Undefined5E, Sleep,

    NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7,
    NumPad8, NumPad9, Multiply, Add, Separator, Subtract, Decimal, Divide,

    F1, F2, F3, F4, F5, F6, F7, F8,
    F9, F10, F11, F12, F13, F14, F15, F16,

    F17, F18, F19, F20, F21, F22, F23, F24,
    Undefined88, Undefined89, Undefined8A, Undefined8B, Undefined8C, Undefined8D, Undefined8E, Undefined8F,

    NumLock, Scroll, OEMFJJisho, OEMFJMasshou, OEMFJTouroku, OEMFJLoya, OEMFJRoya, Undefined97,
    Undefined98, Undefined99, Undefined9A, Undefined9B, Undefined9C, Undefined9D, Undefined9E, Undefined9F,

    LShift, RShift, LControl, RControl, LMenu, RMenu, BrowserBack, BrowserForward,
    BrowserRefresh, BrowserStop, BrowserSearch, BrowserFavorites, BrowserHome, VolumeMute, VolumeDown, VolumeUp,

    MediaNextTrack, MediaPrevTrack, MediaStop, MediaPlayPause, LaunchMail, LaunchMediaSelect, LaunchApp1, LaunchApp2,
    UndefinedB8, UndefinedB9, OEM1, OEMPlus, OEMComma, OEMMinus, OEMPeriod, OEM2,

    OEM3, ABNTC1, ABNTC2, UndefinedC3, UndefinedC4, UndefinedC5, UndefinedC6, UndefinedC7,
    UndefinedC8, UndefinedC9, UndefinedCA, UndefinedCB, UndefinedCC, UndefinedCD, UndefinedCE, UndefinedCF,

    UndefinedD0, UndefinedD1, UndefinedD2, UndefinedD3, UndefinedD4, UndefinedD5, UndefinedD6, UndefinedD7,
    UndefinedD8, UndefinedD9, UndefinedDA, OEM4, OEM5, OEM6, OEM7, OEM8,

    UndefinedE0, OEMAX, OEM102, ICOHelp, ICO00, ProcessKey, ICOClear, Packet,
    UndefinedE8, OEMReset, OEMJump, OEMPA1, OEMPA2, OEMPA3, OEMWsCtrl, OEMCuSel,

    OEMAttn, OEMFinish, OEMCopy, OEMAuto, OEMEnLw, OEMBackTab, Attn, CrSel,
    ExSel, ErEOF, Play, Zoom, NoName, PA1, OEMClear, None
};
