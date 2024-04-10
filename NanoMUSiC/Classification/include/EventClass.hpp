#ifndef EVENT_CLASS_HPP
#define EVENT_CLASS_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <zlib.h>

#include "../../MUSiC/external/msgpack.hpp"
#include "Shifts.hpp"

/** Compress a STL string using zlib with given compression level and return
 * the binary data. */
inline std::string compress_string(const std::string &str, int compressionlevel = Z_BEST_COMPRESSION)
{
    z_stream zs; // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw(std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = (Bytef *)str.data();
    zs.avail_in = str.size(); // set the z_stream's input

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out)
        {
            // append the block to the output string
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
    { // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

/** Decompress an STL string using zlib and return the original data. */
inline std::string decompress_string(const std::string &str)
{
    z_stream zs; // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw(std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = (Bytef *)str.data();
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out)
        {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END)
    { // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ") " << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

// Function to compress data using gzip and save to a file
inline void writeCompressedFile(const std::string &filename, const std::vector<uint8_t> &data)
{
    std::string compressedData = compress_string(std::string(data.begin(), data.end()));
    std::ofstream outputFile(filename, std::ios::out | std::ios::binary);
    if (!outputFile.is_open())
    {
        throw std::runtime_error("Failed to open file for writing.");
    }
    outputFile.write(compressedData.data(), compressedData.size());
    outputFile.close();
}

// Function to read compressed data from file and decompress
inline std::vector<uint8_t> readCompressedFile(const std::string &filename)
{
    std::ifstream inputFile(filename, std::ios::in | std::ios::binary);
    if (!inputFile.is_open())
    {
        throw std::runtime_error("Failed to open file for reading.");
    }
    inputFile.seekg(0, std::ios::end);
    std::streamsize fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    std::string readData(fileSize, ' ');
    inputFile.read(readData.data(), fileSize);
    inputFile.close();

    std::string decompressed_str = decompress_string(readData);
    return std::vector<uint8_t>(decompressed_str.begin(), decompressed_str.end());
}

struct EventClassHistogram
{
    constexpr static float bin_size = 10.;
    constexpr static std::size_t expected_max_bins = 1300;

    std::string name;
    bool weighted;
    std::unordered_map<unsigned int, double> counts;
    std::unordered_map<unsigned int, double> squared_weights;

    auto static make_event_class_histogram(const std::string &name = "", bool weighted = false)
        -> EventClassHistogram;
    auto push(float x, float w = 1.f) -> void;
    auto count(float x) -> double;
    auto error(float x) -> double;
    auto size() -> std::size_t;

    auto bin_index(float x) -> std::size_t;
    auto bounded_bin_index(float x) -> std::optional<std::size_t>;

    template <class T>
    void pack(T &pack)
    {
        pack(name, weighted, counts, squared_weights);
    }
};

// Define fmt::format for EventClassHistogram
template <>
struct fmt::formatter<EventClassHistogram>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const EventClassHistogram &h, FormatContext &ctx)
    {
        std::string out = "[";
        for (auto &&[idx, count] : h.counts)
        {
            if (h.weighted)
            {
                out += fmt::format("({} -> {}, {})", idx, count, h.squared_weights.at(idx));
            }
            else
            {
                out += fmt::format("({} -> {})", idx, count);
            }
        }
        out += "]";
        return format_to(ctx.out(), "{}: {}", h.name, out);
    }
};

struct EventClass
{
    constexpr static auto total_variations = static_cast<std::size_t>(Shifts::Variations::kTotalVariations);

    std::string ec_name;
    std::array<EventClassHistogram, total_variations> h_sum_pt;
    std::array<EventClassHistogram, total_variations> h_invariant_mass;
    std::array<EventClassHistogram, total_variations> h_met;

    static auto make_event_class(const std::string &ec_name) -> EventClass;

    auto histogram(const std::string &observable, std::size_t variation) -> EventClassHistogram &;
    auto sum_pt(std::size_t variation) -> EventClassHistogram &;
    auto invariant_mass(std::size_t variation) -> EventClassHistogram &;
    auto met(std::size_t variation) -> EventClassHistogram &;

    template <typename T>
    auto push(float sum_pt_value,
              float invariant_mass_value,
              const std::optional<float> &met_value,
              float weight,
              T _variation) -> void
    {
        auto variation = static_cast<std::size_t>(_variation);
        sum_pt(variation).push(sum_pt_value, weight);
        invariant_mass(variation).push(invariant_mass_value, weight);
        if (met_value)
        {
            met(variation).push(*met_value, weight);
        }
    }

    template <typename T>
    auto push(float sum_pt_value, float invariant_mass_value, float weight, T variation) -> void
    {
        push(sum_pt_value, invariant_mass_value, std::nullopt, weight, variation);
    }

    inline auto size() -> std::size_t
    {
        return total_variations;
    };

    template <class T>
    void pack(T &pack)
    {
        pack(ec_name, h_sum_pt, h_invariant_mass, h_met);
    }
};

struct EventClassContainer
{
    std::unordered_map<std::string, EventClass> classes;

    auto unsafe_ec(const std::string &ec_name) -> EventClass &
    {
        return classes[ec_name];
    }

    auto ec(const std::string &ec_name) -> EventClass &
    {
        return classes.at(ec_name);
    }

    auto has_ec(const std::string &ec_name) -> bool
    {
        return not(classes.find(ec_name) == classes.end());
    }

    auto push(const std::string &ec_name) -> void
    {
        classes[ec_name] = EventClass::make_event_class(ec_name);
    }

    static auto serialize(EventClassContainer &cont) -> std::string
    {
        auto data = msgpack::pack(cont);
        return compress_string(std::string(data.begin(), data.end()));
    }

    template <class T>
    void pack(T &pack)
    {
        pack(classes);
    }
};

#endif // !EVENT_CLASS_HPP
