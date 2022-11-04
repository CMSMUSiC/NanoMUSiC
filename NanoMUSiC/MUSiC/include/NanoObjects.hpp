#ifndef MUSIC_NANOOBJECTS
#define MUSIC_NANOOBJECTS

#include <any>
#include <iostream>
#include <sstream>
#include <string>

#include "Math/Vector4D.h"

namespace NanoObject
{
using namespace ROOT::Math;

constexpr double MUON_MASS = 105.6583755 / 1000.0;

class NanoObject
{
  public:
    NanoObject(float _pt = 0, float _eta = 0, float _phi = 0, float _mass = 0,
               std::map<std::string, std::any> _features = {})
        : p4(PtEtaPhiMVector(_pt, _eta, _phi, _mass)), features(_features)
    {
    }
    // NanoObject(NanoObject &&) = default;
    // NanoObject(const NanoObject &) = default;
    // NanoObject &operator=(NanoObject &&) = default;
    // NanoObject &operator=(const NanoObject &) = default;
    // ~NanoObject()
    // {
    // }

    // custom members
    PtEtaPhiMVector p4;
    std::map<std::string, std::any> features;

    template <typename T>
    void set(std::string &&feature_name, T feature_value)
    {
        features[feature_name] = feature_value;
    }

    template <typename TypeToCast>
    TypeToCast get(std::string feature_name)
    {
        if (features.find(feature_name) != features.end())
        {
            return std::any_cast<TypeToCast>(features.at(feature_name));
        }
        else
        {
            throw std::runtime_error("The request feature (" + feature_name + ") is not set for this object.");
        }
    }

    unsigned int index()
    {
        if (features.find("index") != features.end())
        {
            return get<unsigned int>("index");
        }
        else
        {
            return -1;
        }
    }

    float pt() const
    {
        return p4.pt();
    }
    float eta() const
    {
        return p4.eta();
    }
    float phi() const
    {
        return p4.phi();
    }
    float mass() const
    {
        return p4.mass();
    }
    float e() const
    {
        return p4.e();
    }

    friend auto operator<<(std::ostream &os, const NanoObject &m) -> std::ostream &
    {
        os << "NanoObject: " << m.p4;
        return os;
    }
};

// factory function
template <typename... Args>
NanoObject make_object(float &&pt, float &&eta, float &&phi, float &&mass, std::pair<const char *, Args> &&...features)
{
    auto _features = std::make_tuple(features...);
    std::map<std::string, std::any> _buffer;
    std::apply([&](auto &&...args) { ((_buffer[args.first] = args.second), ...); }, _features);

    return NanoObject{pt, eta, phi, mass, _buffer};
}

// met-like object
template <typename... Args>
NanoObject make_object(float &&pt, float &&phi, std::pair<const char *, Args> &&...features)
{
    return make_object(std::move(pt), std::move(0.0), std::move(phi), std::move(0.0), std::move(features)...);
}

// event-wise object
template <typename... Args>
NanoObject make_object(std::pair<const char *, Args> &&...features)
{
    return make_object(std::move(0.0), std::move(0.0), std::move(0.0), std::move(0.0), std::move(features)...);
}

// NanoObjects
using NanoObjectCollection = std::vector<NanoObject>;
using NanoAODObjects_t = std::tuple<NanoObjectCollection, /*Muons*/
                                    NanoObjectCollection, /*Electrons*/
                                    NanoObjectCollection, /*Photons*/
                                    NanoObjectCollection, /*Taus*/
                                    NanoObjectCollection, /*BJets*/
                                    NanoObjectCollection, /*Jets*/
                                    NanoObject /*MET*/>;

template <typename ResType, typename F, typename G, typename H>
std::vector<ResType> Where(const NanoObjectCollection &vec, F &&conditional_pred, G &&if_true_pred, H &&if_false_pred)
{
    std::vector<ResType> _out(vec.size());
    std::transform(vec.cbegin(), vec.cend(), _out.begin(), [&](const auto &item) {
        if (conditional_pred(item))
        {
            return if_true_pred(item);
        }
        return if_false_pred(item);
    });
    return _out;
}

template <typename F>
std::vector<int> BuildMask(const NanoObjectCollection &vec, F &&pred)
{
    std::vector<int> _out(vec.size());
    std::transform(vec.cbegin(), vec.cend(), _out.begin(), [&](const auto &item) {
        if (pred(item))
        {
            return 1;
        }
        return 0;
    });
    return _out;
}

template <typename F>
NanoObjectCollection Filter(NanoObjectCollection &vec, F &&pred)
{
    NanoObjectCollection _out;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(_out), pred);
    return _out;
}

template <typename F>
NanoObjectCollection Filter(const NanoObjectCollection &vec, F &&pred)
{
    NanoObjectCollection _out;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(_out), pred);
    return _out;
}

template <typename F>
NanoObjectCollection Filter(NanoObjectCollection &&vec, F &&pred)
{
    NanoObjectCollection _out;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(_out), pred);
    return _out;
}

NanoObjectCollection Filter(const NanoObjectCollection &vec, std::vector<int> &conditions)
{
    if (vec.size() != conditions.size())
    {
        throw std::runtime_error("Conditions vector is shorter than NanoObjectCollection.");
    }

    NanoObjectCollection _out;
    _out.reserve(vec.size());
    for (unsigned int i = 0; i < vec.size(); i++)
    {
        if (conditions.at(i))
        {
            _out.emplace_back(vec.at(i));
        }
    }
    _out.shrink_to_fit();
    return _out;
}

NanoObjectCollection Take(const NanoObjectCollection &vec, const std::vector<int> &indices)
{
    if (vec.size() < indices.size())
    {
        throw std::runtime_error("Indices vector is larger than NanoObjectCollection.");
    }

    NanoObjectCollection _out;
    _out.reserve(vec.size());
    for (unsigned int i = 0; i < indices.size(); i++)
    {
        _out.emplace_back(vec.at(indices.at(i)));
    }
    _out.shrink_to_fit();
    return _out;
}

template <typename T>
std::vector<T> GetFeature(NanoObjectCollection &collection, std::string feature_name)
{
    std::vector<T> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.get<T>(feature_name));
    }
    return _buffer;
}

template <typename T>
void SetFeature(NanoObjectCollection &collection, std::string &&feature_name, std::vector<T> feature_values)
{
    for (unsigned int i = 0; i < feature_values.size(); i++)
    {
        collection[i].set<T>(std::move(feature_name), feature_values[i]);
    }
}

std::vector<unsigned int> Indices(NanoObjectCollection &collection)
{
    return GetFeature<unsigned int>(collection, "index");
}

NanoObject GetByIndex(NanoObjectCollection &collection, unsigned int _index)
{
    // TODO: use binary search
    for (auto &obj : collection)
    {
        if (obj.index() == _index)
        {
            return obj;
        }
    }
    return NanoObject();
}

template <typename T>
auto Repr(const std::vector<T> &vec)
{
    std::ostringstream _buff;
    _buff << "{ ";
    std::for_each(vec.cbegin(), vec.cend(), [&](const auto &item) {
        _buff << "[ ";
        _buff << item;
        _buff << " ]";
    });
    _buff << " }";
    return _buff.str();
}

auto Repr(NanoObjectCollection &collection)
{
    std::ostringstream _buff;
    _buff << "{ ";
    std::for_each(collection.cbegin(), collection.cend(), [&](auto &obj) {
        _buff << "[ ";
        _buff << obj;
        _buff << " ]";
    });
    _buff << " }";
    return _buff.str();
}

std::vector<float> Pt(NanoObjectCollection &collection)
{
    std::vector<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.pt());
    }
    return _buffer;
}

std::vector<float> Eta(NanoObjectCollection &collection)
{
    std::vector<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.eta());
    }
    return _buffer;
}

std::vector<float> Phi(NanoObjectCollection &collection)
{
    std::vector<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.phi());
    }
    return _buffer;
}

std::vector<float> Mass(NanoObjectCollection &collection)
{
    std::vector<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.mass());
    }
    return _buffer;
}

std::vector<float> E(NanoObjectCollection &collection)
{
    std::vector<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.e());
    }
    return _buffer;
}

template <typename T>
void _unroll_and_fill(std::vector<std::map<std::string, std::any>> &_features_buffer,
                      std::pair<const char *, std::vector<T>> &&features)
{
    for (unsigned int i = 0; i < features.second.size(); i++)
    {
        _features_buffer[i][features.first] = features.second[i];
    }
}

// factory function (particle-like)
template <typename... Args>
NanoObjectCollection make_collection(std::vector<float> &&pt, std::vector<float> &&eta, std::vector<float> &&phi,
                                     std::vector<float> &&mass,
                                     std::pair<const char *, std::vector<Args>> &&...features)
{
    auto _features = std::make_tuple(features...);
    auto _features_buffer = std::vector<std::map<std::string, std::any>>(pt.size());
    std::apply([&](auto &&...args) { (_unroll_and_fill(_features_buffer, std::move(args)), ...); }, _features);

    NanoObjectCollection _tmp_collection;
    for (size_t i = 0; i < pt.size(); i++)
    {
        _tmp_collection.emplace_back(pt.at(i), eta.at(i), phi.at(i), mass.at(i), _features_buffer.at(i));
    }

    auto _indices = std::vector<unsigned int>(_tmp_collection.size());
    std::iota(_indices.begin(), _indices.end(), 0);
    SetFeature(_tmp_collection, "index", _indices);

    return _tmp_collection;
}

// mass as constant (value)
template <typename... Args>
NanoObjectCollection make_collection(std::vector<float> &&pt, std::vector<float> &&eta, std::vector<float> &&phi,
                                     float mass, std::pair<const char *, Args> &&...features)
{
    return make_collection(std::move(pt), std::move(eta), std::move(phi),
                           std::move(std::vector<float>(pt.size(), mass)), std::move(features)...);
}

} // namespace NanoObject

#endif /*MUSIC_NANOOBJECTS*/
