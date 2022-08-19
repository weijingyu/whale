#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace whale {

	class Log
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
#define WH_CORE_TRACE(...)    ::whale::Log::getCoreLogger()->trace(__VA_ARGS__)
#define WH_CORE_INFO(...)     ::whale::Log::getCoreLogger()->info(__VA_ARGS__)
#define WH_CORE_WARN(...)     ::whale::Log::getCoreLogger()->warn(__VA_ARGS__)
#define WH_CORE_ERROR(...)    ::whale::Log::getCoreLogger()->error(__VA_ARGS__)
#define WH_CORE_CRITICAL(...) ::whale::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define WH_TRACE(...)         ::whale::Log::getClientLogger()->trace(__VA_ARGS__)
#define WH_INFO(...)          ::whale::Log::getClientLogger()->info(__VA_ARGS__)
#define WH_WARN(...)          ::whale::Log::getClientLogger()->warn(__VA_ARGS__)
#define WH_ERROR(...)         ::whale::Log::getClientLogger()->error(__VA_ARGS__)
#define WH_CRITICAL(...)      ::whale::Log::getClientLogger()->critical(__VA_ARGS__)

