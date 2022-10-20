
#include <any>
#include <string>

#include "Math/Vector4D.h"
#include "ROOT/RVec.hxx"

namespace NanoObject
{
using namespace ROOT::Math;
using namespace ROOT::VecOps;

const double MUON_MASS = 105.6583755 / 1000.0;

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

    template <class T>
    void set(std::string &&feature_name, T feature_value)
    {
        features[feature_name] = feature_value;
    }

    template <class TypeToCast>
    TypeToCast get(std::string feature_name)
    {
        if (features.find(feature_name) != features.end())
        {
            return std::any_cast<TypeToCast>(features[feature_name]);
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

    float pt()
    {
        return p4.pt();
    }
    float eta()
    {
        return p4.eta();
    }
    float phi()
    {
        return p4.phi();
    }
    float mass()
    {
        return p4.mass();
    }
    float e()
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
template <class... Args>
NanoObject make_object(float &&pt, float &&eta, float &&phi, float &&mass, std::pair<const char *, Args> &&...features)
{
    auto _features = std::make_tuple(features...);
    std::map<std::string, std::any> _buffer;
    std::apply([&](auto &&...args) { ((_buffer[args.first] = args.second), ...); }, _features);

    return NanoObject{pt, eta, phi, mass, _buffer};
}

// met-like object
template <class... Args>
NanoObject make_object(float &&pt, float &&phi, std::pair<const char *, Args> &&...features)
{
    return make_object(std::move(pt), std::move(0.0), std::move(phi), std::move(0.0), std::move(features)...);
}

// event-wise object
template <class... Args>
NanoObject make_object(std::pair<const char *, Args> &&...features)
{
    return make_object(std::move(0.0), std::move(0.0), std::move(0.0), std::move(0.0), std::move(features)...);
}

// NanoObjects
typedef RVec<NanoObject> NanoObjectCollection;

template <typename F>
NanoObjectCollection Filter(const NanoObjectCollection &vec, F &&pred)
{
    NanoObjectCollection _out;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(_out), pred);
    return _out;
}

NanoObjectCollection Filter(const NanoObjectCollection &vec, RVec<int> condictions)
{
    NanoObjectCollection _out;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(_out), [&condictions](auto nanoobj) { return true; });
    return _out;
}

template <class T>
RVec<T> GetFeature(NanoObjectCollection &collection, std::string &&feature_name)
{
    RVec<T> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.get<T>(feature_name));
    }
    return _buffer;
}

template <class T>
void SetFeature(NanoObjectCollection &collection, std::string &&feature_name, RVec<T> feature_values)
{
    for (unsigned int i = 0; i < feature_values.size(); i++)
    {
        collection[i].set<T>(std::move(feature_name), feature_values[i]);
    }
}

RVec<unsigned int> Indices(NanoObjectCollection &collection)
{
    return GetFeature<unsigned int>(collection, "index");
}

NanoObject GetByIndex(NanoObjectCollection &collection, unsigned int _index)
{
    for (auto &obj : collection)
    {
        if (obj.index() == _index)
        {
            return obj;
        }
    }
    return NanoObject();
}

RVec<float> Pt(NanoObjectCollection &collection)
{
    RVec<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.pt());
    }
    return _buffer;
}

RVec<float> Eta(NanoObjectCollection &collection)
{
    RVec<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.eta());
    }
    return _buffer;
}

RVec<float> Phi(NanoObjectCollection &collection)
{
    RVec<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.phi());
    }
    return _buffer;
}

RVec<float> Mass(NanoObjectCollection &collection)
{
    RVec<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.mass());
    }
    return _buffer;
}

RVec<float> E(NanoObjectCollection &collection)
{
    RVec<float> _buffer;
    for (auto &obj : collection)
    {
        _buffer.emplace_back(obj.e());
    }
    return _buffer;
}

template <class T>
void _unroll_and_fill(RVec<std::map<std::string, std::any>> &_features_buffer,
                      std::pair<const char *, RVec<T>> &&features)
{
    for (unsigned int i = 0; i < features.second.size(); i++)
    {
        _features_buffer[i][features.first] = features.second[i];
    }
}

// factory function (particle-like)
template <class... Args>
NanoObjectCollection make_collection(RVec<float> &&pt, RVec<float> &&eta, RVec<float> &&phi, RVec<float> &&mass,
                                     std::pair<const char *, RVec<Args>> &&...features)
{
    auto _features = std::make_tuple(features...);
    auto _features_buffer = RVec<std::map<std::string, std::any>>(pt.size());
    std::apply([&](auto &&...args) { (_unroll_and_fill(_features_buffer, std::move(args)), ...); }, _features);

    auto _tmp_collection = Construct<NanoObject>(pt, eta, phi, mass, _features_buffer);

    auto _indices = RVec<unsigned int>(_tmp_collection.size());
    std::iota(_indices.begin(), _indices.end(), 0);
    SetFeature(_tmp_collection, "index", _indices);

    return _tmp_collection;
}

// mass as constant (value)
template <class... Args>
NanoObjectCollection make_collection(RVec<float> &&pt, RVec<float> &&eta, RVec<float> &&phi, float mass,
                                     std::pair<const char *, Args> &&...features)
{
    return make_collection(std::move(pt), std::move(eta), std::move(phi), std::move(RVec<float>(pt.size(), mass)),
                           std::move(features)...);
}

} // namespace NanoObject