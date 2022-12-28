#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
class Enumerate
{
  private:
    const std::vector<T> categories;

  public:
    const std::size_t total;

    Enumerate(const std::initializer_list<T> &_list) : categories({_list}), total(_list.size())
    {
        // check for duplicates
        std::vector<T> _vec = categories;
        std::sort(_vec.begin(), _vec.end());
        if (std::unique(_vec.begin(), _vec.end()) != _vec.end())
        {
            throw std::runtime_error("The provided list has duplicates.");
        }
    }

    using const_iterator = typename std::vector<T>::const_iterator;

    const_iterator begin() const
    {
        return categories.cbegin();
    }

    const_iterator end() const
    {
        return categories.cend();
    }

    const_iterator cbegin() const
    {
        return categories.cbegin();
    }

    const_iterator cend() const
    {
        return categories.cend();
    }

    std::size_t size() const
    {
        return categories.size();
    }

    T operator[](const std::size_t index) const
    {
        return categories[index];
    }

    template <typename Q = T, std::enable_if_t<!std::is_integral<Q>::value, bool> = true>
    std::size_t operator[](const Q &str) const
    {
        auto it = std::find(this->categories.cbegin(), this->categories.cend(), str);
        return std::distance(this->categories.cbegin(), it);
    }

    std::size_t index_of(const T &str) const
    {
        auto it = std::find(this->categories.cbegin(), this->categories.cend(), str);
        if (it == this->categories.cend())
        {
            if constexpr (std::is_constructible_v<std::string, T>)
            {
                throw std::runtime_error("Item \"" + std::string(str) + "\" not found.");
            }
            else
            {
                throw std::runtime_error("Item \"" + std::to_string(str) + "\" not found.");
            }
        }
        else
        {
            return std::distance(this->categories.cbegin(), it);
        }
    }

    T at(const std::size_t index) const
    {
        return categories.at(index);
    }

    friend std::ostream &operator<<(std::ostream &os, const Enumerate &_enum)
    {
        os << "[";
        for (std::size_t i = 0; i < _enum.categories.size(); i++)
        {
            os << _enum.categories[i];
            if (i < _enum.categories.size() - 1)
            {
                os << " - ";
            }
        }
        os << "]";
        return os;
    }
};

void enumerate_test()
{
    const auto arr = Enumerate{"foo", "bar", "oi"};
    // const Enumerate arr = {"foo", "bar", "oi"};
    // const Enumerate arr = {10.2, 20.2, 30.2, 40.2, 60.2};
    // const Enumerate arr = {10, 20, 30, 40, 60};
    fmt::print("\n--> arr: {} - size: {}\n", arr, arr.size());
    fmt::print("\n--> Op[]: {} \n", arr["bar"]);
    // fmt::print("\n--> Index of: {} \n", arr[20]);
    fmt::print("\n--> Op[]: {} \n", arr[2]);

    fmt::print("\n--> Index of: {} \n", arr.index_of("foo"));
    // fmt::print("\n--> Index of: {} \n", arr.at(3));
    fmt::print("\n--> at: {} \n", arr.at(2));
    fmt::print("\n--> Total: {} \n", arr.total);

    for (const auto &item : arr)
    {
        std::cout << "Loop: " << item << "\n";
    }
}
