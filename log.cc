#include "log.h"
namespace ekko{
template<class T>
Fmt::Fmt(const char* fmt, T val) {
    static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
    snprintf(buf_, sizeof(buf_), fmt, val);
}

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);

#define MAXNUMERICSIZE 64
const char digitsHex[] = "0123456789ABCDEF";
template<typename T>
void LogStream::formatInteger(T v) {
if (buffer_.avail() >= MAXNUMERICSIZE) {
    size_t len = convert(buffer_.current(), v);
    buffer_.add(len);
}
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

size_t convertHex(char buf[], uintptr_t value){
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

LogStream& LogStream::operator<<(const void* p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.avail() >= MAXNUMERICSIZE)
    {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf+2, v);
        buffer_.add(len+2);
    }
    return *this;
}

LogStream& LogStream::operator<<(double v)
{
    if (buffer_.avail() >= MAXNUMERICSIZE)
    {
        int len = snprintf(buffer_.current(), MAXNUMERICSIZE, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

/*
 Format a number with 5 characters, including SI units.
 [0,     999]
 [1.00k, 999k]
 [1.00M, 999M]
 [1.00G, 999G]
 [1.00T, 999T]
 [1.00P, 999P]
 [1.00E, inf)
*/
std::string formatSI(int64_t s)
{
    double n = static_cast<double>(s);
    char buf[64];
    if (s < 1000)
        snprintf(buf, sizeof(buf), "%ld", s);
    else if (s < 9995)
        snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
    else if (s < 99950)
        snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
    else if (s < 999500)
        snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
    else if (s < 9995000)
        snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
    else if (s < 99950000)
        snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
    else if (s < 999500000)
        snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
    else if (s < 9995000000)
        snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
    else if (s < 99950000000)
        snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
    else if (s < 999500000000)
        snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
    else if (s < 9995000000000)
        snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
    else if (s < 99950000000000)
        snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
    else if (s < 999500000000000)
        snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
    else if (s < 9995000000000000)
        snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
    else if (s < 99950000000000000)
        snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
    else if (s < 999500000000000000)
        snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
    else
        snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
    return buf;
}

/*
 [0, 1023]
 [1.00Ki, 9.99Ki]
 [10.0Ki, 99.9Ki]
 [ 100Ki, 1023Ki]
 [1.00Mi, 9.99Mi]
*/
std::string formatIEC(int64_t s)
{
    double n = static_cast<double>(s);
    char buf[64];
    const double Ki = 1024.0;
    const double Mi = Ki * 1024.0;
    const double Gi = Mi * 1024.0;
    const double Ti = Gi * 1024.0;
    const double Pi = Ti * 1024.0;
    const double Ei = Pi * 1024.0;

    if (n < Ki)
        snprintf(buf, sizeof buf, "%ld", s);
    else if (n < Ki*9.995)
        snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
    else if (n < Ki*99.95)
        snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
    else if (n < Ki*1023.5)
        snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

    else if (n < Mi*9.995)
        snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
    else if (n < Mi*99.95)
        snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
    else if (n < Mi*1023.5)
        snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

    else if (n < Gi*9.995)
        snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
    else if (n < Gi*99.95)
        snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
    else if (n < Gi*1023.5)
        snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

    else if (n < Ti*9.995)
        snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
    else if (n < Ti*99.95)
        snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
    else if (n < Ti*1023.5)
        snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

    else if (n < Pi*9.995)
        snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
    else if (n < Pi*99.95)
        snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
    else if (n < Pi*1023.5)
        snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

    else if (n < Ei*9.995)
        snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
    else
        snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
    return buf;
}

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel() {
    if (::getenv("EKKO_LOG_TRACE"))
        return Logger::TRACE;
    else if (::getenv("EKKO_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
    s.append(v.data_, v.size_);
    return s;
}

void defaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
}

void defaultFlush()
{
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
  : time_(Timestamp::now()),
    stream_(),
    level_(level),
    line_(line),
    basename_(file)
{
    formatTime();
    stream_ << t_this_thread->tid();
    stream_ << LogLevelName[level];
    if (savedErrno != 0)
    {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct DateTime dt;
        dt = BreakTime(seconds);

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
            dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        assert(len == 17);
    }
    Fmt us(".%06dZ ", microseconds);
    assert(us.length() == 9);
    stream_ << t_time << us.data();
}

void Logger::Impl::finish()
{
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort?FATAL:ERROR, errno, file, line)
{
}

Logger::~Logger()
{
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL)
    {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}
}