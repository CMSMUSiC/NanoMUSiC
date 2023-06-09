#ifndef EVENT_HPP
#define EVENT_HPP

#include "EventView.hpp"

class Event {
    using EventViewMap_t =
        std::unordered_map<std::string_view, EventView>::iterator;
    using ConstEventViewMap_t =
        std::unordered_map<std::string_view, EventView>::const_iterator;

    std::unordered_map<std::string_view, EventView> m_event_views;

   public:
    Event() : m_event_views() {}
    Event(EventView &&event_view) : m_event_views() {
        m_event_views.emplace("nominal", event_view);
    }

    auto begin() -> EventViewMap_t { return m_event_views.begin(); }

    auto end() -> EventViewMap_t { return m_event_views.end(); }

    auto cbegin() -> const ConstEventViewMap_t {
        return m_event_views.cbegin();
    }

    auto cend() -> const ConstEventViewMap_t { return m_event_views.cend(); }

    auto set_view(const std::string_view &view_name, EventView &&event_view)
        -> Event & {
        m_event_views.emplace(view_name, event_view);
        return *this;
    }

    auto set_view(EventView &&event_view) -> Event & {
        m_event_views.emplace("nominal", event_view);
        return *this;
    }

    auto add_view(const std::string_view &view_name, EventView &&event_view)
        -> void {
        m_event_views.emplace(view_name, event_view);
    }

    auto add_view(EventView &&event_view) -> void {
        m_event_views.emplace("nominal", event_view);
    }

    auto get_view(const std::string_view &view_name) -> EventView & {
        return m_event_views.at(view_name);
    }

    auto get_views() -> const std::vector<std::string_view> {
        std::vector<std::string_view> view_names;
        for (const auto &[key, _] : m_event_views) {
            view_names.push_back(key);
        }

        return view_names;
    }
};

#endif