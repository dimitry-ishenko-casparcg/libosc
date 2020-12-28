////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "packet.hpp"
#include "value.hpp"

#include <algorithm>
#include <stdexcept>

#include <endian.h>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

////////////////////////////////////////////////////////////////////////////////
namespace
{

int32 padded(int32 x) { return ((x + 3) / 4) * 4; }

int32 space_(int32 ) { return sizeof(int32 ); }
int32 space_(float ) { return sizeof(float ); }
int32 space_(const string& s) { return padded(s.size() + 1); }
int32 space_(const blob& b) { return space_(int32{}) + padded(b.size()); }
int32 space_(int64 ) { return sizeof(int64 ); }
int32 space_(time  ) { return sizeof(int64 ); } // sent as int64
int32 space_(double) { return sizeof(double); }
int32 space_(char  ) { return sizeof(int32 ); } // sent as int32
int32 space_(bool  ) { return 0; }
int32 space_(null  ) { return 0; }
int32 space_(inf_t ) { return 0; }

}

////////////////////////////////////////////////////////////////////////////////
int32 value::space() const
{
         if(is_int32 ()) return space_( to_int32 ());
    else if(is_float ()) return space_( to_float ());
    else if(is_string()) return space_( to_string());
    else if(is_blob  ()) return space_( to_blob  ());
    else if(is_int64 ()) return space_( to_int64 ());
    else if(is_time  ()) return space_( to_time  ());
    else if(is_double()) return space_( to_double());
    else if(is_char  ()) return space_( to_char  ());
    else if(is_bool  ()) return space_( to_bool  ());
    else if(is_nil   ()) return space_( to_nil   ());
    else if(is_inf   ()) return space_( to_inf   ());
    else throw std::invalid_argument("osc::value::space()): invalid type");
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& pkt) const
{
         if(is_int32 ()) append_to(pkt, to_int32 ());
    else if(is_float ()) append_to(pkt, to_float ());
    else if(is_string()) append_to(pkt, to_string());
    else if(is_blob  ()) append_to(pkt, to_blob  ());
    else if(is_int64 ()) append_to(pkt, to_int64 ());
    else if(is_time  ()) append_to(pkt, to_time  ());
    else if(is_double()) append_to(pkt, to_double());
    else if(is_char  ()) append_to(pkt, to_char  ());
    else if(is_bool  ()) append_to(pkt, to_bool  ());
    else if(is_nil   ()) append_to(pkt, to_nil   ());
    else if(is_inf   ()) append_to(pkt, to_inf   ());
    else throw std::invalid_argument("osc::value::append_to(osc::packet&): invalid type");
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, int32 i)
{
    i = htobe32(i);
    p.append(reinterpret_cast<char*>(&i), sizeof(i));
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, float f)
{
    append_to(p, *reinterpret_cast<int32*>(&f));
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, string s)
{
    if(s.find('\0') != std::string::npos) throw std::invalid_argument(
        "osc::value::append_to(packet&, string): '\0' inside string"
    );

    s.resize(padded(s.size() + 1));
    p.append(s.data(), s.size());
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, blob b)
{
    append_to(p, static_cast<int32>(b.size()));
    b.resize(padded(b.size()));
    p.append(b.data(), b.size());
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, int64 i)
{
    i = htobe64(i);
    p.append(reinterpret_cast<char*>(&i), sizeof(i));
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, time t)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    using fractions = duration<int64, std::ratio<1, 0x100000000>>; // 1/(2^32)

    if(t != immed)
    {
        // "shift" epoch from 1/1/1970 (unix) to 1/1/1900 (osc)
        // which is 70 years + 17 leap days
        // ref: https://stackoverflow.com/a/65149566/4358570
        t += (70 * 365 + 17) * 24h;

        auto total = t.time_since_epoch();
        auto sec = duration_cast<seconds>(total);
        auto frac = duration_cast<fractions>(total - sec);

        append_to(p, static_cast<int64>((sec.count() << 32) | frac.count()));
    }
    else append_to(p, static_cast<int64>(1));
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, double d)
{
    append_to(p, *reinterpret_cast<int64*>(&d));
}

////////////////////////////////////////////////////////////////////////////////
void value::append_to(packet& p, char c)
{
    append_to(p, static_cast<int32>(c));
}

//////////////////////////////////////////////////if//////////////////////////////
value value::parse(packet& p, char tag)
{
    switch(tag)
    {
    case 'i': return parse_int32 (p);
    case 'f': return parse_float (p);
    case 's': return parse_string(p);
    case 'b': return parse_blob  (p);
    case 'h': return parse_int64 (p);
    case 't': return parse_time  (p);
    case 'd': return parse_double(p);
    case 'c': return parse_char  (p);
    case 'T': return true;
    case 'F': return false;
    case 'N': return nil;
    case 'I': return inf;
    }

    throw std::invalid_argument("osc::value::parse(packet&, char): invalid tag");
}

////////////////////////////////////////////////////////////////////////////////
int32 value::parse_int32(packet& p)
{
    if(p.data_.size() < sizeof(int32)) throw std::invalid_argument(
        "osc::value::parse_int32(osc::packet&): incomplete packet"
    );

    int32 i = be32toh(*reinterpret_cast<int32*>(p.data_.data()));
    p.data_.erase(p.data_.begin(), p.data_.begin() + sizeof(int32));

    return i;
}

////////////////////////////////////////////////////////////////////////////////
float value::parse_float(packet& p)
{
    auto i = parse_int32(p);
    return *reinterpret_cast<float*>(&i);
}

////////////////////////////////////////////////////////////////////////////////
string value::parse_string(packet& p)
{
    auto end = std::find(p.data_.begin(), p.data_.end(), '\0');
    if(end == p.data_.end()) throw std::invalid_argument(
        "osc::value::parse_string(osc::packet&): missing '\0'"
    );

    string s(p.data_.begin(), end);

    end = p.data_.begin() + padded(end - p.data_.begin() + 1);
    p.data_.erase(p.data_.begin(), end);

    return s;
}

////////////////////////////////////////////////////////////////////////////////
blob value::parse_blob(packet& p)
{
    auto size = parse_int32(p);

    if(p.size() < size) throw std::invalid_argument(
        "osc::value::parse_blob(osc::packet&): incomplete packet"
    );
    auto end = p.data_.begin() + size;

    blob b(p.data_.begin(), end);

    end = p.data_.begin() + padded(end - p.data_.begin());
    p.data_.erase(p.data_.begin(), end);

    return b;
}

//////////////////////////////////////////////////if//////////////////////////////
int64 value::parse_int64(packet& p)
{
    if(p.data_.size() < sizeof(int64)) throw std::invalid_argument(
        "osc::value::parse_int64(osc::packet&): incomplete packet"
    );

    int64 i = be64toh(*reinterpret_cast<int64*>(p.data_.data()));
    p.data_.erase(p.data_.begin(), p.data_.begin() + sizeof(int64));

    return i;
}

//////////////////////////////////////////////////if//////////////////////////////
time value::parse_time(packet& p)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    using fractions = duration<int64, std::ratio<1, 0x100000000>>; // 1/(2^32)

    time t; // epoch

    // "shift" epoch from 1/1/1900 (osc) to 1/1/1970 (unix)
    // which is 70 years + 17 leap days
    // ref: https://stackoverflow.com/a/65149566/4358570
    t -= (70 * 365 + 17) * 24h;

    auto i = parse_int64(p);

    t += seconds(i >> 32);
    t += duration_cast<clock::duration>( fractions(i & 0xffffffff) );

    return t;
}

//////////////////////////////////////////////////if//////////////////////////////
double value::parse_double(packet& p)
{
    auto i = parse_int64(p);
    return *reinterpret_cast<double*>(&i);
}

//////////////////////////////////////////////////if//////////////////////////////
char value::parse_char(packet& p)
{
    auto i = parse_int32(p);
    return static_cast<char>(i);
}

//////////////////////////////////////////////////if//////////////////////////////
}
