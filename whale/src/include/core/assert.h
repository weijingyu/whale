#pragma once

#include "core/base.h"
#include "core/log.h"
#include <filesystem>

#ifdef WH_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define WH_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { WH##type##ERROR(msg, __VA_ARGS__); WH_DEBUGBREAK(); } }
#define WH_INTERNAL_ASSERT_WITH_MSG(type, check, ...) WH_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define WH_INTERNAL_ASSERT_NO_MSG(type, check) WH_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", WH_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define WH_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define WH_INTERNAL_ASSERT_GET_MACRO(...) WH_EXPAND_MACRO( WH_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, WH_INTERNAL_ASSERT_WITH_MSG, WH_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define WH_ASSERT(...) WH_EXPAND_MACRO( WH_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define WH_CORE_ASSERT(...) WH_EXPAND_MACRO( WH_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define WH_ASSERT(...)
#define WH_CORE_ASSERT(...)
#endif