#ifndef BDB_ERROR_HPP_
#define BDB_ERROR_HPP_

#include <cstring>     // std::strerror
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

namespace bdb {

std::string message_with_errno(const std::string_view& message) noexcept;

// A runtime error that may be thrown from within `bdb`.
class Error : public std::runtime_error {
public:
  explicit Error(const std::string_view& message) noexcept;

  static Error with_errno(const std::string_view& message) noexcept;
};

} // namespace bdb

#endif // BDB_ERROR_HPP_
