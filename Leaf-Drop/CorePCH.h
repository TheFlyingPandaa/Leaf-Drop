#pragma once

#include <Windows.h>

#ifdef _DEBUG
#include <iostream>

#define PRINT(p) {std::cout << p << std::endl;}
#else
#define PRINT(p);
#endif
