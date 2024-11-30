#include <libbdb/error.hpp>

namespace bdb {

Error::Error(const std::string_view& message)
    : std::runtime_error{std::string{message}} {}

Error Error::with_errno(const std::string_view& message) {
  return Error{std::string{message} + " [Errno " + std::strerror(errno) + "]"};
}

} // namespace bdb
