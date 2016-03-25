#pragma once

#ifndef RESTC_CPP_HELPER_H_
#define RESTC_CPP_HELPER_H_

#include <memory>
#include <string>
#include <cstring>
#include <boost/utility/string_ref.hpp>

namespace restc_cpp {

// recommended in Meyers, Effective STL when internationalization and embedded
// NULLs aren't an issue.  Much faster than the STL or Boost lex versions.
// Source: http://stackoverflow.com/questions/1801892/making-mapfind-operation-case-insensitive
struct ciLessLibC : public std::binary_function<std::string, std::string, bool> {
    bool operator()(const std::string &lhs, const std::string &rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
    }
};

/*! Merge map dst into map src if dst != nullptr */
template <typename T>
void merge_map(const boost::optional<T> src, T& dst) {
    if (src) {
        for(const auto& it : *src) {
            dst.insert(it);
        }
    }
}


// TODO: See if I can get the buffers from the stream object and
// map it directly to asio buffers, single or multiple fragments.
class ToBuffer
{
public:
    ToBuffer(const std::ostringstream& stream)
    : buf_{stream.str()}
    {
    }

    ToBuffer(std::string&& str)
    : buf_{std::move(str)}
    {
    }

    operator const boost::asio::const_buffers_1 () const {
        return {buf_.c_str(), buf_.size()};
    }

    operator const boost::string_ref() const {
        return buf_;
    }

private:
    const std::string buf_;
};



} // restc_cpp

#endif // RESTC_CPP_HELPER_H_
