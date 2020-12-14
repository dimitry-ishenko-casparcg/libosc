////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef OSC_MESSAGE_HPP
#define OSC_MESSAGE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "types.hpp"
#include "value.hpp"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

class packet;

////////////////////////////////////////////////////////////////////////////////
class message
{
public:
    message(string address);

    auto const& address() const { return address_; }
    auto const& values() const { return values_; }

    message& operator<<(value v)
    {
        values_.push_back(std::move(v));
        return *this;
    }

    int32 space() const;
    packet to_packet() const;

    static bool maybe(packet&);
    static message parse(packet&);

private:
    string address_;
    std::vector<value> values_;

    void append_to(packet&) const;
    friend class element;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
