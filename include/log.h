#pragma once

#include <iostream>
#include "config.h"

#ifdef ENABLE_LOGGER

#define LOG_ERROR   std::cerr << RED << "[ error ]" << "[" << __FUNCTION__ << "]"
#else   
#define LOG_ERROR   while (false) std::cerr
#endif