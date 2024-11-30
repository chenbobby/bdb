#ifndef BDB_ERROR_HPP_
#define BDB_ERROR_HPP_

#include <cstring>     // std::strerror
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

namespace bdb {

// A runtime error that may be thrown from within `bdb`.
class Error : public std::runtime_error {
public:
  Error(const std::string_view& message);

  static Error with_errno(const std::string_view& message);
};

} // namespace bdb

#endif // BDB_ERROR_HPP_
