#ifndef __LOG_H__
#define __LOG_H__
#include <boost/noncopyable.hpp>
#include <string>
#include <cstring>
#include <algorithm>
#include <memory>
#include "lock.h"
#include "timestamp.h"
namespace ekko{
template<int SIZE>
class FixedBuffer : boost::noncopyable {
public:
    FixedBuffer() { cur_ = buf_;}

    size_t length() const { return cur_ - buf_}

    const char* data() const { return buf_;}

    void add(int x) { cur_ += x;}

    void reset() { cur_ = buf_;}

    size_t avail() { return SIZE - length();}

    void append(const char* buf, size_t len) {
        if (len < avail()) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    template<int X>
    int append(const FixedBuffer<X>& buf) {
        if (buf.length() > avail())
            return -1;
        memcpy(cur, buf.buf_, buf.length());
        cur += buf.length();
        return 0;
    }

    char* current() const { return cur_;}
private:
    char buf_[SIZE];
    char *cur_;
};

const int SMALLBUFFERSIZE = 4000;
const int BIGBUFFERSIZE = 4000*1000;

class Fmt : boost::noncopyable{
public:
    template<class T> Fmt(const char* fmt, T val);

    const char* data() const { return buf_;}
    int length() const { return length();}
private:
    char buf_[32];
    int length_;
};

class LogStream : boost::noncopyable {
public:
    typedef LogStream self;
    typedef FixedBuffer<SMALLBUFFERSIZE> Buffer;

    self& operator<<(bool v) {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    self& operator<<(char v) {
        buffer_.append(&v, 1);
        return *this;
    }


    self& operator<<(const char* str)
    {
        if (str)
        {
        buffer_.append(str, strlen(str));
        }
        else
        {
        buffer_.append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    self& operator<<(const Buffer& v)
    {
        buffer_.append(v);
        return *this;
    }

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }
private:
    template<class T>
    void formatInteger(T v);

    Buffer buffer_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
    s.append(fmt.data(), fmt.length());
    return s;
}

std::string formatSI(int64_t n);

std::string formatIEC(int64_t n);


class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    // compile time calculation of basename of source file
    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char (&arr)[N])
        : data_(arr),
            size_(N-1) {
            const char* slash = strrchr(data_, '/'); // builtin function
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename)
        : data_(filename) {
            const char* slash = strrchr(filename, '/');
            if (slash)
            {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char* data_;
        int size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    //static void setTimeZone(const TimeZone& tz);

private:

    class Impl {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };

    Impl impl_;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}


class LogFile : boost::noncopyable {
public:
    LogFile(const std::string& basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    static std::string getLogFileName(const std::string& basename, time_t* now);

    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    std::unique_ptr<Mutex> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    //std::unique_ptr<FileUtil::AppendFile> file_;

    const static int kRollPerSeconds_ = 60*60*24;
};
}
#endif