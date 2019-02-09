#pragma once

// Initialize paging. This functions is meant to be called
// from boot.asm, before any static constructors.
extern "C" void initializePaging();
