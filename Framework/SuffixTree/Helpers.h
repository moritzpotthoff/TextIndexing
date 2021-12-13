#pragma once

#include <iostream>
#include <cassert>

#define AssertMsg(assumption, message) assert((assumption) || (std::cout << "ASSERTION FAILED: " << message << "File: " << __FILE__ << "\nLine: " << __LINE__ << "\n" << std::flush && false))
