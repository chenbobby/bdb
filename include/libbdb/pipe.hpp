#ifndef BDB_PIPE_HPP_
#define BDB_PIPE_HPP_

#include <cstddef>
#include <unistd.h>
#include <vector>

#include <libbdb/error.hpp>

namespace bdb {

// A pipe for sending a string once, from on process to another.
class Pipe {
public:
  explicit Pipe(const bool should_close_on_exec);
  ~Pipe() noexcept;

  // Sends a string into the pipe.
  void send(const std::string_view& str);

  // Retrieves a string from the pipe.
  std::string receive();

  void close_sender();
  void close_receiver();

private:
  int _file_descriptors[2];
  bool _should_close_on_exec;
};

} // namespace bdb

#endif // BDB_PIPE_HPP_
