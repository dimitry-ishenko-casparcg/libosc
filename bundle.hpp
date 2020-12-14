////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef OSC_BUNDLE_HPP
#define OSC_BUNDLE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "message.hpp"
#include "packet.hpp"
#include "types.hpp"

#include <variant>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

////////////////////////////////////////////////////////////////////////////////
class bundle
{
public:
    class element;

    bundle(osc::time t = clock::now()) : time_(std::move(t)) { }

    auto const& time() const { return time_; }
    auto const& elements() const { return elements_; }

    bundle& operator<<(element);

    int32 space() const;
    packet to_packet() const;

private:
    osc::time time_;
    std::vector<element> elements_;

    void append_to(packet&) const;
};

////////////////////////////////////////////////////////////////////////////////
class bundle::element
{
public:
    element(message m) : element_(std::move(m)) { }
    element(bundle  b) : element_(std::move(b)) { }

    bool is_message() const { return std::holds_alternative<message>(element_); }
    bool is_bundle () const { return std::holds_alternative<bundle >(element_); }

    auto const& to_message() const { return std::get<message>(element_); }
    auto const& to_bundle () const { return std::get<bundle >(element_); }

    int32 space() const;

private:
    std::variant<message, bundle> element_;

    void append_to(packet&) const;
    friend class bundle;
};

////////////////////////////////////////////////////////////////////////////////
inline bundle& bundle::operator<<(element e)
{
    elements_.push_back(std::move(e));
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
