#include <libbdb/error.hpp>

namespace bdb {

std::string message_with_errno(const std::string_view& message) noexcept {
  return std::string{message} + ": errno: " + std::strerror(errno);
}

Error::Error(const std::string_view& message) noexcept
    : std::runtime_error{std::string{message}} {}

Error Error::with_errno(const std::string_view& message) noexcept {
  return Error{message_with_errno(message)};
}

} // namespace bdb
