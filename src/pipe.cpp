#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <libbdb/error.hpp>
#include <libbdb/pipe.hpp>
#include <vector>

namespace bdb {

Pipe::Pipe(const bool should_close_on_exec)
    : _should_close_on_exec{should_close_on_exec} {
  auto pipe_flags{this->_should_close_on_exec ? O_CLOEXEC : 0};
  if (pipe2(this->_file_descriptors, pipe_flags) < 0) {
    throw Error::with_errno("failed to create pipe");
  }
}

Pipe::~Pipe() noexcept {
  this->close_sender();
  this->close_receiver();
}

std::string Pipe::receive() {
  char buffer[128];
  if (read(this->_file_descriptors[0], buffer, sizeof(buffer)) < 0) {
    throw Error::with_errno("failed to read from pipe");
  }
  return std::string{buffer};
}

void Pipe::send(const std::string_view& str) {
  const auto buffer{std::string{str}};
  if (write(this->_file_descriptors[1], buffer.c_str(), str.size()) < 0) {
    throw Error::with_errno("failed to send through pipe");
  }
}

void Pipe::close_sender() {
  if (this->_file_descriptors[1] != -1) {
    if (close(this->_file_descriptors[1]) < 0) {
      throw Error::with_errno("failed to close pipe sender");
    }
    this->_file_descriptors[1] = -1;
  }
}

void Pipe::close_receiver() {
  if (this->_file_descriptors[0] != -1) {
    if (close(this->_file_descriptors[0]) < 0) {
      throw Error::with_errno("failed to close pipe receiver");
    }
    this->_file_descriptors[0] = -1;
  }
}

} // namespace bdb
