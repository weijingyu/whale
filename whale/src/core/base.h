#pragma once

#ifdef WH_PLATFORM_WINDOWS
#ifdef WH_BUILD_DLL
#define WH_API __declspec(dllexport)
#else
#define WH_API __declspec(dllimport)
#endif
#endif