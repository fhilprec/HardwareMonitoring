#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <string>
#include <stdarg.h>

namespace util{

/**
 * A simple timer using the systems steady clock.
 */
class Timer{
private:
    // Type aliases to make accessing nested type easier
    using clock_t = std::chrono::steady_clock;
    using ms_t = std::chrono::duration<double, std::ratio<1,1000>>;

    std::chrono::time_point<clock_t> m_beg;

public:
    Timer() : m_beg(clock_t::now()){}

    void reset(){
        m_beg = clock_t::now();
    }

    double elapsed() const{
        return std::chrono::duration_cast<ms_t>(clock_t::now() - m_beg).count();
    }

    static void sleep_ms(uint32_t ms){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static uint64_t time_seed(){
        return (uint64_t) std::chrono::duration_cast<ms_t>(clock_t::now().time_since_epoch()).count();
    }
};

template<typename T>
class Singleton{
protected:
    Singleton() noexcept = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    virtual ~Singleton() = default; // to silence base class Singleton<T> has a
    // non-virtual destructor [-Weffc++]

public:
    static T& get() noexcept(std::is_nothrow_constructible<T>::value){
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        // Thread safe in C++11
        static T instance{};

        return instance;
    }
};



/**
 * Simple Logger.
 */
struct Log : Singleton<Log> {
    enum LogLevel{
        INFO,
        WARN,
        ERROR
    };
    /**
     * The current loglevel.
     * To use DEBUG, compile in debug mode.
     */
    LogLevel lvl = INFO;

    void debug(std::string str){
        (void) str;
        (void) lvl;
#ifndef NDEBUG
        std::cout << "#[DEBUG] " << str << '\n';
#endif
    }
    void debug_fmt(const char* format...){
#ifndef NDEBUG
        std::cout << "#[DEBUG] ";
        va_list argptr;
        va_start(argptr, format);
        vprintf(format, argptr);
        va_end(argptr);
        std::cout << '\n';
#endif
    }
    void info(std::string str){
        log(INFO, str);
    }
    void info_fmt(const char* format...){
        std::cout << "#[INFO ] ";
        va_list argptr;
        va_start(argptr, format);
        vprintf(format, argptr);
        va_end(argptr);
        std::cout << '\n';
    }
    void warn(std::string str){
        log(WARN, str);
    }
    void warn_fmt(const char* format...){
        std::cout << "#[WARN ] ";
        va_list argptr;
        va_start(argptr, format);
        vprintf(format, argptr);
        va_end(argptr);
        std::cout << '\n';
    }
    void error(std::string str){
        log(ERROR, str);
    }
    void error_fmt(const char* format...){
        std::cout << "#[ERROR] ";
        va_list argptr;
        va_start(argptr, format);
        vprintf(format, argptr);
        va_end(argptr);
        std::cout << '\n';
    }
    /**
     * Can be used directly, but will produce code for debug level!
     */
    void log(LogLevel level, std::string str){
        if(level >= lvl) std::cout << l2s(level) << str << '\n';
    }
private:
    inline static std::string l2s(LogLevel level) {
        switch(level) {
            case INFO:  return "#[INFO ] ";
            case WARN:  return "#[WARN ] ";
            case ERROR: return "#[ERROR] ";
            default:    return "#[what?] ";
        }
    }
};


/**
 * Threadpool
 */
class ThreadPool {
public:
    /**
     * Example Usage:
     * ThreadPool::parallel_n(8, [&](int tid) {
     *     std::cout << "Hello, here is thread " << tid << '\n';
     * });
     */
    template <typename Fn>
    static void parallel_n(int n, Fn&& fn) {
        std::vector<std::thread> threads;
        threads.reserve(n);
        for (int i = 0; i < n; ++i) {
            threads.emplace_back(std::thread(fn, i));
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    // static only class, disallow instances
    ThreadPool() = delete;
};

static std::vector<std::string> str_split(std::string s, std::string delim, bool allow_empty = false) {
    std::vector<std::string> found;
    size_t prev = 0, pos = 0, delim_len = delim.length();
    do{
        pos = s.find(delim, prev);
        if (pos == std::string::npos) pos = s.length();
        std::string token = s.substr(prev, pos-prev);
        if (!token.empty() || allow_empty) found.push_back(token);
        prev = pos + delim_len;
    }while (pos < s.length() && prev < s.length());

    return found;
}

static uint64_t get_proc_info(std::string type, std::string which){
    std::ifstream in("/proc/self/"+type);
    std::string line;
    while(std::getline(in,line)){
        auto parts = str_split(line,": ");
        if (parts[0] == which){
            return std::stoull(parts[1]);
        }
    }
    return (uint64_t) -1;
}

} // end of namespace
