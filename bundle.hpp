////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef OSC_BUNDLE_HPP
#define OSC_BUNDLE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "types.hpp"

#include <variant>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

class element;
class packet;

////////////////////////////////////////////////////////////////////////////////
class bundle
{
public:
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
    friend class element;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
