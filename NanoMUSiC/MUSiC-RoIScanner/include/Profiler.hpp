#ifndef PROFILER_HH
#define PROFILER_HH

#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>
#include <sys/time.h>

// rapidjson-support
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include "allocators.h"
#include "document.h"
#pragma GCC diagnostic pop

// usage:
// Profiler< CpuClock > profiler;

class CpuClock
{
  public:
    static double getTime()
    {
        return (double)std::clock() / CLOCKS_PER_SEC;
    }
};

class ChronoClock
{
  public:
    static double getTime()
    {
        const std::chrono::time_point<std::chrono::high_resolution_clock> now =
            std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> duration = now.time_since_epoch();
        return duration.count();
    }
};

class WallClock
{
  public:
    static double getTime()
    {
        struct timeval now;
        if (gettimeofday(&now, NULL))
        {
            return 0;
        }
        return ((double)now.tv_sec + (double)now.tv_usec * 1e-6);
    }
};

// maybe we want to use float later
typedef double seconds;

template <class ClockType> class ClockProfiler
{
  public:
    ClockProfiler(std::string name = "Unnamed")
        : profilerName(name), startTime(0), durationCount(0), durationDeviation(0), durationTotal(0),
          durationMin(INFINITY), durationMax(-INFINITY)
    {
    }

    inline seconds getTime()
    {
        return ClockType::getTime();
    }

    void start()
    {
        startTime = getTime();
    }

    double stop()
    {
        const seconds stopTime = getTime();
        const seconds duration = stopTime - startTime;

        const seconds oldMean = durationTotal / durationCount;
        durationCount++;
        durationTotal += duration;
        const seconds newMean = durationTotal / durationCount;

        durationMin = std::min(durationMin, duration);
        durationMax = std::max(durationMax, duration);

        if (durationCount == 1)
        {
            durationDeviation = 0;
        }
        else
        {
            durationDeviation += (duration - oldMean) * (duration - newMean);
        }
        return duration;
    }

    seconds min() const
    {
        return (durationCount >= 1) ? durationMin : 0;
    }

    seconds max() const
    {
        return (durationCount >= 1) ? durationMax : 0;
    }

    seconds mean() const
    {
        return (durationCount >= 1) ? durationTotal / durationCount : 0;
    }

    seconds std() const
    {
        return (durationCount > 1) ? sqrt(durationDeviation / (durationCount - 1)) : 0;
    }

    seconds sum() const
    {
        return durationTotal;
    }

    size_t count() const
    {
        return durationCount;
    }

    std::string name() const
    {
        return profilerName;
    }

    void clear()
    {
        startTime = 0;
        durationCount = 0;
        sum = 0;
        durationDeviation = 0;
    }

    void print() const
    {
        std::cout << (*this) << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &stream, const ClockProfiler &profiler)
    {
        const std::streamsize precision = stream.precision();
        const std::ios_base::fmtflags flags = stream.flags();

        stream.precision(3);
        // stream << std::scientific;

        stream << "Profiler '" << profiler.name() << "':" << std::endl;
        stream << "\tCount: " << profiler.count() << std::endl;
        stream << "\tTotal: " << profiler.sum() << " sec" << std::endl;
        stream << "\tMean:  " << profiler.mean() << " sec" << std::endl;
        stream << "\tStd:   " << profiler.std() << " sec" << std::endl;
        stream << "\tMin:   " << profiler.min() << " sec" << std::endl;
        stream << "\tMax:   " << profiler.max() << " sec" << std::endl;

        stream.precision(precision);
        stream.flags(flags);
        return stream;
    }

    // this function provides support for rapidjson
    // it can be completely removed if this support is not wanted anymore
    rapidjson::Value rapidjsonValue(rapidjson::Document::AllocatorType &allocator) const
    {
        rapidjson::Value value(rapidjson::kObjectType);
        value.AddMember("name", name(), allocator);
        value.AddMember("count", count(), allocator);
        value.AddMember("total", sum(), allocator);
        value.AddMember("mean", mean(), allocator);
        value.AddMember("std", std(), allocator);
        value.AddMember("min", min(), allocator);
        value.AddMember("max", max(), allocator);
        return value;
    }

  private:
    std::string profilerName;
    seconds startTime;
    unsigned long long durationCount;
    seconds durationDeviation;
    seconds durationTotal;
    seconds durationMin;
    seconds durationMax;
};

typedef ClockProfiler<ChronoClock> Profiler;

#endif // PROFILER_HH
