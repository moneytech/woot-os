#pragma once

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(_min, _max, val) (max((_min), min((_max), (val))))
#define align(val, alignment) ((alignment) * (((val) + ((alignment) - 1)) / (alignment)))
#define swap(T, a, b) { T t = (a); (a) = (b); (b) = (t); }
