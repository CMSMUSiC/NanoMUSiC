#ifndef ENUMERATE_H
#define ENUMERATE_H

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace Enumerate
{

template <typename T>
constexpr int _imp2_number_of_duplicates(T &&t1, T &&t2)
{
    return (t1 == t2 ? 1 : 0);
}

template <typename T, typename... Args>
constexpr int _imp1_number_of_duplicates(T &&t, Args &&...args)
{
    return (_imp2_number_of_duplicates(std::forward<T>(t), std::forward<Args>(args)) + ...);
}

template <typename... Args>
constexpr int number_of_duplicates(Args &&...args)
{
    constexpr std::size_t N = sizeof...(Args);
    return (_imp1_number_of_duplicates(std::forward<Args>(args), std::forward<Args>(args)...) + ...) - N;
}

template <typename... T>
class Enumerate
{
    static_assert(sizeof...(T) > 0, "Must have at least one element.");
    constexpr static std::size_t N = sizeof...(T);
    using CT = std::common_type_t<T...>;

  public:
    std::array<CT, N> categories;

    constexpr Enumerate(T &&...values)
        : categories(std::array<CT, sizeof...(T)>{values...})
    {
        // static_assert(number_of_duplicates(values...) == 0,
        //               "Must not have duplicated elements.");
    }

    using const_iterator = typename std::array<CT, N>::const_iterator;

    constexpr const_iterator begin() const
    {
        return categories.cbegin();
    }

    constexpr const_iterator end() const
    {
        return categories.cend();
    }

    constexpr const_iterator cbegin() const
    {
        return categories.cbegin();
    }

    constexpr const_iterator cend() const
    {
        return categories.cend();
    }

    constexpr std::size_t size() const
    {
        return categories.size();
    }

    constexpr CT operator[](const std::size_t index) const
    {
        return categories[index];
    }

    template <typename Q = CT, std::enable_if_t<!std::is_integral<Q>::value, bool> = true>
    constexpr std::size_t operator[](const Q &str) const
    {
        auto it = std::find(this->categories.cbegin(), this->categories.cend(), str);
        return std::distance(this->categories.cbegin(), it);
    }

    constexpr std::size_t index_of(const CT &val) const
    {
        auto it = std::find(this->categories.cbegin(), this->categories.cend(), val);
        if (it == this->categories.cend())
        {
            if constexpr (std::is_constructible_v<std::string, CT>)
            {
                throw std::runtime_error("Item \"" + std::string(val) + "\" not found.");
            }
            else
            {
                throw std::runtime_error("Item \"" + std::to_string(val) + "\" not found.");
            }
        }
        else
        {
            return std::distance(this->categories.cbegin(), it);
        }
    }

    constexpr CT at(const std::size_t index) const
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

template <typename... Ts>
constexpr auto make_enumerate(Ts &&...args)
{
    static_assert(sizeof...(Ts) > 0, "Must have at least one element.");

    // if constexpr (number_of_duplicates(args...) == 0,
    //               "Must not have duplicated elements.");

    using FirstEntityType = std::tuple_element_t<0, std::tuple<Ts...>>;
    if constexpr (std::is_convertible_v<FirstEntityType, std::string_view>)
    {
        return Enumerate(std::string_view(std::forward<Ts>(args))...);
    }
    else
    {
        return Enumerate(std::forward<Ts>(args)...);
    }
}

} // namespace Enumerate
#endif /*ENUMERATE_H*/