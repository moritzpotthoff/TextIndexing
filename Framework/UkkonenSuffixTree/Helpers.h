#pragma once

#include <iostream>
#include <cassert>

#define AssertMsg(assumption, message) assert((assumption) || (std::cout << "ASSERTION FAILED: " << message << "\n" << std::flush && false))
