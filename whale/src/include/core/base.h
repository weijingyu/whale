#pragma once

#include <memory>

namespace Whale {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#ifdef WHALE_DEBUG
	#if defined(WH_PLATFORM_WINDOWS)
		#define WH_DEBUGBREAK() __debugbreak()
	#elif defined(WH_PLATFORM_LINUX)
		#include <signal.h>
		#define WH_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
#define WH_ENABLE_ASSERTS
#else
	#define WH_DEBUGBREAK()
#endif

#define WH_EXPAND_MACRO(x) x
#define WH_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define WH_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#include <core/log.h>
#include <core/assert.h>