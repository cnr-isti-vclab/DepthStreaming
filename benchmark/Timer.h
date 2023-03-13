#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>

namespace DStream
{
    struct ProfileResult
    {
        std::string Name;
        long long Start, End;
        size_t ThreadID;
    };

    struct TimerSession
    {
        std::string Name;
    };

    class TimerManager
    {
    private:
        TimerSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        int m_ProfileCount;
    public:
        TimerManager()
            : m_CurrentSession(nullptr), m_ProfileCount(0)
        {
        }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
            m_OutputStream.open(filepath);
            m_ProfileCount = 0;
            WriteHeader();
            m_CurrentSession = new TimerSession{ name };
        }

        void EndSession()
        {
            WriteFooter();
            m_OutputStream.close();
            delete m_CurrentSession;
            m_CurrentSession = nullptr;
            m_ProfileCount = 0;
        }

        void WriteProfile(const ProfileResult& result)
        {
            if (m_ProfileCount++ > 0)
                m_OutputStream << ",";

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutputStream << "{";
            m_OutputStream << "\"cat\":\"function\",";
            m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
            m_OutputStream << "\"name\":\"" << name << "\",";
            m_OutputStream << "\"ph\":\"X\",";
            m_OutputStream << "\"pid\":0,";
            m_OutputStream << "\"tid\":" << result.ThreadID << ",";
            m_OutputStream << "\"ts\":" << result.Start;
            m_OutputStream << "}";

            m_OutputStream.flush();
        }

        inline void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
            m_OutputStream.flush();
        }

        inline void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        inline static TimerManager& Get()
        {
            static TimerManager instance;
            return instance;
        }
    };

    class Timer
    {
    public:
        Timer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        ~Timer()
        {
            if (!m_Stopped)
                Stop();
        }

        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
            TimerManager::Get().WriteProfile({ m_Name, start, end, threadID });

            m_Stopped = true;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        bool m_Stopped;
    };
}

#define DSTR_PROFILE    0

#if DSTR_PROFILE
#define DSTR_PROFILE_BEGIN_SESSION(name, filepath) ::DStream::TimerManager::Get().BeginSession(name,filepath)
#define DSTR_PROFILE_END_SESSION() ::DStream::TimerManager::Get().EndSession()
#define DSTR_PROFILE_SCOPE(name) ::DStream::Timer timer##__LINE__(name);
#define DSTR_PROFILE_FUNCTION() DSTR_PROFILE_SCOPE(__FUNCSIG__)
#else
#define DSTR_PROFILE_BEGIN_SESSION(name, filepath)
#define DSTR_PROFILE_END_SESSION()
#define DSTR_PROFILE_FUNCTION()
#define DSTR_PROFILE_SCOPE(name)
#endif