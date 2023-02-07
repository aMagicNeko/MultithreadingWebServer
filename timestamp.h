#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__
#include <boost/operators.hpp>
#include <string>
#include <memory>
#include <vector>
#include <ctime>
namespace ekko{

class Timestamp : public boost::equality_comparable<Timestamp>, public boost::less_than_comparable<Timestamp> {
public:
    Timestamp() : microSecondsSinceEpoch_(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpochArg) : microSecondsSinceEpoch_(microSecondsSinceEpochArg) {}

    void swap(Timestamp& that) {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    static Timestamp now();
    static Timestamp invalid() {
        return Timestamp();
    }

    static Timestamp fromUnixTime(time_t t) {
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(time_t t, int microseconds) {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

struct DateTime
{
    DateTime() {}
    explicit DateTime(const struct tm&);
    DateTime(int _year, int _month, int _day, int _hour, int _minute, int _second)
        : year(_year), month(_month), day(_day), hour(_hour), minute(_minute), second(_second){}

    // "2022-12-31 12:34:56"
    std::string toIsoString() const;

    int year = 0;     // [1900, 2500]
    int month = 0;    // [1, 12]
    int day = 0;      // [1, 31]
    int hour = 0;     // [0, 23]
    int minute = 0;   // [0, 59]
    int second = 0;   // [0, 59]
};

inline void fillHMS(unsigned seconds, struct DateTime* dt)
{
  dt->second = seconds % 60;
  unsigned minutes = seconds / 60;
  dt->minute = minutes % 60;
  dt->hour = minutes / 60;
}
const int kSecondsPerDay = 24*60*60;

struct Data
{
    struct Transition
    {
        int64_t utctime;
        int64_t localtime;  // Shifted Epoch
        int localtimeIdx;

        Transition(int64_t t, int64_t l, int localIdx)
            : utctime(t), localtime(l), localtimeIdx(localIdx)
        { }
    };

    struct LocalTime
    {
        int32_t utcOffset;  // East of UTC
        bool isDst;
        int desigIdx;

        LocalTime(int32_t offset, bool dst, int idx)
            : utcOffset(offset), isDst(dst), desigIdx(idx)
        { }
    };
    void addLocalTime(int32_t utcOffset, bool isDst, int desigIdx)
    {
        localtimes.push_back(LocalTime(utcOffset, isDst, desigIdx));
    }

    void addTransition(int64_t utcTime, int localtimeIdx)
    {
        LocalTime lt = localtimes.at(localtimeIdx);
        transitions.push_back(Transition(utcTime, utcTime + lt.utcOffset, localtimeIdx));
    }

    const LocalTime* findLocalTime(int64_t utcTime) const;
    const LocalTime* findLocalTime(const struct DateTime& local, bool postTransition) const;

    struct CompareUtcTime
    {
        bool operator()(const Transition& lhs, const Transition& rhs) const
        {
        return lhs.utctime < rhs.utctime;
        }
    };
    struct CompareLocalTime
    {
        bool operator()(const Transition& lhs, const Transition& rhs) const
        {
        return lhs.localtime < rhs.localtime;
        }
    };

    std::vector<Transition> transitions;
    std::vector<LocalTime> localtimes;
    std::string abbreviation;
    std::string tzstring;
};

///
/// Date in Gregorian calendar.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Date
{
public:
    struct YearMonthDay
    {
        int year; // [1900..2500]
        int month;  // [1..12]
        int day;  // [1..31]
    };

    static const int kDaysPerWeek = 7;
    static const int kJulianDayOf1970_01_01;

    ///
    /// Constucts an invalid Date.
    ///
    Date()
        : julianDayNumber_(0)
    {}

    ///
    /// Constucts a yyyy-mm-dd Date.
    ///
    /// 1 <= month <= 12
    Date(int year, int month, int day);

    ///
    /// Constucts a Date from Julian Day Number.
    ///
    explicit Date(int julianDayNum)
        : julianDayNumber_(julianDayNum)
    {}

    ///
    /// Constucts a Date from struct tm
    ///
    explicit Date(const struct tm&);

    // default copy/assignment/dtor are Okay

    void swap(Date& that)
    {
        std::swap(julianDayNumber_, that.julianDayNumber_);
    }

    bool valid() const { return julianDayNumber_ > 0; }

    ///
    /// Converts to yyyy-mm-dd format.
    ///
    std::string toIsoString() const;

    struct YearMonthDay yearMonthDay() const;

    int year() const
    {
        return yearMonthDay().year;
    }

    int month() const
    {
        return yearMonthDay().month;
    }

    int day() const
    {
        return yearMonthDay().day;
    }

    // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
    int weekDay() const
    {
        return (julianDayNumber_+1) % kDaysPerWeek;
    }

    int julianDayNumber() const { return julianDayNumber_; }

private:
    int julianDayNumber_;
};

inline bool operator<(Date x, Date y)
{
    return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator==(Date x, Date y)
{
    return x.julianDayNumber() == y.julianDayNumber();
}

DateTime BreakTime(int64_t t);
}

#endif