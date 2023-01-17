#ifndef CONVOLUTIONLOOKUP_HH
#define CONVOLUTIONLOOKUP_HH

#define TABLE_VERSION 1

#include <cstddef> // for size_t
#include <cstdint> // for the fixed size int types

#include <string>

#include "ConvolutionComputer.hpp"

// https://github.com/bshoshany/thread-pool
#include "BS_thread_pool.hpp"

#include <chrono>
// https://github.com/p-ranav/indicators
#include "indicators.hpp"
#include <thread>

typedef struct
{
    // "private" entries
    uint8_t _version;
    uint8_t _sizeof_float;
    uint32_t _count;

    // "public" entries
    int prior;
    size_t data_npoints;

    float data_lower_block_size;
    unsigned int data_log_block_start;
    float data_log_block_factor;

    float bg_factor_down;
    float bg_factor_up;
    size_t bg_npoints_down;
    size_t bg_npoints_up;
    float uncert_min;

    float uncert_factor;
    size_t uncert_npoints;

} __attribute__((packed)) LookupOptions;

class LookupTable
{
  public:
    LookupTable(bool debug = false);
    ~LookupTable();

    void generate(const LookupOptions &options);
    double lookup(double data, double bg, double uncert) const;

    void writeFile(std::string filename = "");
    void readFile(std::string filename = "");

    size_t expectedBinarySize() const;

    bool isLoaded() const;
    std::string lastLoadedFilename() const;

    void printPoints() const;

    static std::string getDefaultLutPath();

  private:
    void calcPoints(double data_index, double bg_index, double uncert_index, double &data_out, double &bg_out,
                    double &uncert_out) const;
    void calcIndices(double data, double bg, double uncert, double &data_index_out, double &bg_index_out,
                     double &uncert_index_out) const;

    double interpolate(double data_index, double bg_index, double uncert_index) const;

    inline size_t totalDataPoints() const;
    inline size_t totalBgPoints() const;
    inline size_t totalUncertPoints() const;
    inline size_t totalPoints() const;
    inline PriorMode prior() const;

    static double calcFactor(double index, double factor_down, double factor_up);
    static double unCalcFactor(double factor, double factor_down, double factor_up);

    void newTable(const float default_value = -1.);
    void deleteTable();

    inline size_t index(size_t data_index, size_t bg_index, size_t uncert_index) const;

    const bool debug = false;
    LookupOptions options;
    float *buffer = nullptr;
    std::string lastFile;

    static constexpr float OUT_OF_BOUNDS = -1.;
    static constexpr float INVALID_P = -2.;
    static constexpr float UNTRUSTED_P = -3.;
    static constexpr float UNINITIALIZED = -4.;
    static constexpr float BETWEEN_REGIONS = -5.;
};

extern "C" double lookup_p_convolution(double N_obs, double N_SM, double error_parameter, bool debug = false,
                                       const char *filename = "");

#endif // CONVOLUTIONLOOKUP_HH
