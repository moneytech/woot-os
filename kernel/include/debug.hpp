#pragma once

#include <stream.hpp>

extern Stream *DebugStream;

#define DEBUG DebugStream->WriteFmt
