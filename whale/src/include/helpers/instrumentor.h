#pragma once

#include "core/log.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

namespace Whale {

	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string name;

		FloatingPointMicroseconds start;
		std::chrono::microseconds elapsedTime;
		std::thread::id threadID;
	};

	struct InstrumentationSession
	{
		std::string name;
	};

	class Instrumentor
	{
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void beginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			std::lock_guard lock(m_mutex);
			if (m_currentSession)
			{
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				if (Log::getCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
				{
					WH_CORE_ERROR("Instrumentor::beginSession('{0}') when session '{1}' already open.", name, m_currentSession->name);
				}
				internalEndSession();
			}
			m_outputStream.open(filepath);

			if (m_outputStream.is_open())
			{
				m_currentSession = new InstrumentationSession({ name });
				writeHeader();
			}
			else
			{
				if (Log::getCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
				{
					WH_CORE_ERROR("Instrumentor could not open results file '{0}'.", filepath);
				}
			}
		}

		void endSession()
		{
			std::lock_guard lock(m_mutex);
			internalEndSession();
		}

		void writeProfile(const ProfileResult& result)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.elapsedTime.count()) << ',';
			json << "\"name\":\"" << result.name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.threadID << ",";
			json << "\"ts\":" << result.start.count();
			json << "}";

			std::lock_guard lock(m_mutex);
			if (m_currentSession)
			{
				m_outputStream << json.str();
				m_outputStream.flush();
			}
		}

		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}
	private:
		Instrumentor()
			: m_currentSession(nullptr)
		{
		}

		~Instrumentor()
		{
			endSession();
		}

		void writeHeader()
		{
			m_outputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			m_outputStream.flush();
		}

		void writeFooter()
		{
			m_outputStream << "]}";
			m_outputStream.flush();
		}

		// Note: you must already own lock on m_mutex before
		// calling InternalEndSession()
		void internalEndSession()
		{
			if (m_currentSession)
			{
				writeFooter();
				m_outputStream.close();
				delete m_currentSession;
				m_currentSession = nullptr;
			}
		}
	private:
		std::mutex m_mutex;
		InstrumentationSession* m_currentSession;
		std::ofstream m_outputStream;
	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name)
			: m_name(name), m_stopped(false)
		{
			m_startTimepoint = std::chrono::steady_clock::now();
		}

		~InstrumentationTimer()
		{
			if (!m_stopped)
				stop();
		}

		void stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = FloatingPointMicroseconds{ m_startTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch();

			Instrumentor::Get().writeProfile({ m_name, highResStart, elapsedTime, std::this_thread::get_id() });

			m_stopped = true;
		}
	private:
		const char* m_name;
		std::chrono::time_point<std::chrono::steady_clock> m_startTimepoint;
		bool m_stopped;
	};

	namespace InstrumentorUtils {

		template <size_t N>
		struct ChangeResult
		{
			char data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;
				if (matchIndex == K - 1)
					srcIndex += matchIndex;
				result.data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	}
}

#define WH_PROFILE 1
#if WH_PROFILE
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define WH_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define WH_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define WH_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define WH_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define WH_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define WH_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define WH_FUNC_SIG __func__
#else
#define WH_FUNC_SIG "WH_FUNC_SIG unknown!"
#endif

#define WH_PROFILE_BEGIN_SESSION(name, filepath) ::Whale::Instrumentor::Get().BeginSession(name, filepath)
#define WH_PROFILE_END_SESSION() ::Whale::Instrumentor::Get().endSession()
#define WH_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Whale::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Whale::InstrumentationTimer timer##line(fixedName##line.data)
#define WH_PROFILE_SCOPE_LINE(name, line) WH_PROFILE_SCOPE_LINE2(name, line)
#define WH_PROFILE_SCOPE(name) WH_PROFILE_SCOPE_LINE(name, __LINE__)
#define WH_PROFILE_FUNCTION() WH_PROFILE_SCOPE(WH_FUNC_SIG)
#else
#define WH_PROFILE_BEGIN_SESSION(name, filepath)
#define WH_PROFILE_END_SESSION()
#define WH_PROFILE_SCOPE(name)
#define WH_PROFILE_FUNCTION()
#endif