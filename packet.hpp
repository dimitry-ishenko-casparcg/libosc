////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef OSC_PACKET_HPP
#define OSC_PACKET_HPP

////////////////////////////////////////////////////////////////////////////////
#include "types.hpp"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

////////////////////////////////////////////////////////////////////////////////
class packet
{
public:
    packet() = default;
    packet(const char* data, int32 size) : packet(data, data + size) { }
    packet(const char* begin, const char* end) : data_(begin, end) { }

    const char* data() const { return data_.data(); }
    int32 size() const { return data_.size(); }

private:
    std::vector<char> data_;
    void append(const char*, int32);

    friend class value;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
