#ifndef BDB_TRACEE_HPP_
#define BDB_TRACEE_HPP_

#include <cstdint>    // std::uint8_t
#include <filesystem> // std::filesystem::path
#include <memory>     // std::unique_ptr
#include <ostream>
#include <sys/types.h> // pid_t

namespace bdb {

enum class TraceeState {
  STOPPED,
  RUNNING,
  EXITED,
  TERMINATED,
};

// An event that is created when a tracee is stopped.
class TraceeStoppedEvent {
public:
  TraceeStoppedEvent(const pid_t& pid, const int wait_status);

  TraceeState tracee_state() const { return this->_tracee_state; }
  void print(std::ostream& out);

private:
  pid_t _pid;
  TraceeState _tracee_state;
  std::uint8_t _info;
};

class Tracee {
public:
  Tracee()
      : _pid{0}, _state{TraceeState::STOPPED}, _should_terminate_session_on_end{
                                                   false} {}

  Tracee(const pid_t& pid, const bool should_terminate_session_on_end)
      : _pid{pid}, _state{TraceeState::STOPPED},
        _should_terminate_session_on_end{should_terminate_session_on_end} {}

  /* Disable copy construction and copy assignment */

  Tracee(const Tracee&) = delete;
  Tracee& operator=(const Tracee&) = delete;

  ~Tracee();

  /* Factory methods */

  // Constructs a `Tracee` by executing a new program at `path`.
  static std::unique_ptr<Tracee> launch(const std::filesystem::path& path);

  // Constructs a `Tracee` by attaching to an existing PID.
  static std::unique_ptr<Tracee> attach(const pid_t& pid);

  /* Instance methods */

  // Resume execution of the tracee.
  void resume();

  // Wait on the tracee. Return a `TraceeStoppedEvent` when the tracee is
  // stopped.
  TraceeStoppedEvent wait_on_signal();

  pid_t pid() const;

private:
  pid_t _pid;
  TraceeState _state;
  bool _should_terminate_session_on_end;
};

} // namespace bdb

#endif // BDB_TRACEE_HPP_
