#pragma once

#include "base.h"
#include "spdlog/spdlog.h"
#include <spdlog/fmt/ostr.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Whale {

	class WH_API Log
	{
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_clientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	};

}

// Core log macros
#define WH_CORE_TRACE(...)    ::Whale::Log::getCoreLogger()->trace(__VA_ARGS__)
#define WH_CORE_INFO(...)     ::Whale::Log::getCoreLogger()->info(__VA_ARGS__)
#define WH_CORE_WARN(...)     ::Whale::Log::getCoreLogger()->warn(__VA_ARGS__)
#define WH_CORE_ERROR(...)    ::Whale::Log::getCoreLogger()->error(__VA_ARGS__)
#define WH_CORE_CRITICAL(...) ::Whale::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define WH_TRACE(...)         ::Whale::Log::getClientLogger()->trace(__VA_ARGS__)
#define WH_INFO(...)          ::Whale::Log::getClientLogger()->info(__VA_ARGS__)
#define WH_WARN(...)          ::Whale::Log::getClientLogger()->warn(__VA_ARGS__)
#define WH_ERROR(...)         ::Whale::Log::getClientLogger()->error(__VA_ARGS__)
#define WH_CRITICAL(...)      ::Whale::Log::getClientLogger()->critical(__VA_ARGS__)

