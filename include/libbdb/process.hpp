#ifndef BDB_PROCESS_HPP_
#define BDB_PROCESS_HPP_

#include <cstdint>    // std::uint8_t
#include <filesystem> // std::filesystem::path
#include <memory>     // std::unique_ptr
#include <ostream>
#include <sys/types.h> // pid_t

namespace bdb {

enum class ProcessState {
  STOPPED,
  RUNNING,
  EXITED,
  TERMINATED,
};

// An event that is created when the inferior processes is stopped.
class ProcessStoppedEvent {
public:
  ProcessStoppedEvent(const pid_t& pid, const int wait_status);

  ProcessState process_state() const { return this->_process_state; }
  void print(std::ostream& out);

private:
  pid_t _pid;
  ProcessState _process_state;
  std::uint8_t _info;
};

class Process {
public:
  Process()
      : _pid{0}, _state{ProcessState::STOPPED},
        _should_terminate_session_on_end{false} {}

  Process(const pid_t& pid, const bool should_terminate_session_on_end)
      : _pid{pid}, _state{ProcessState::STOPPED},
        _should_terminate_session_on_end{should_terminate_session_on_end} {}

  /* Disable copy construction and copy assignment */

  Process(const Process&) = delete;
  Process& operator=(const Process&) = delete;

  ~Process();

  /* Factory methods */

  // Constructs a `Process` by executing a new program at `path`.
  static std::unique_ptr<Process> launch(const std::filesystem::path& path);

  // Constructs a `Process` by attaching to an existing process.
  static std::unique_ptr<Process> attach(const pid_t& pid);

  /* Instance methods */

  // Resume execution of the inferior process.
  void resume();

  // Wait on the inferior process. Return a `ProcessStoppedEvent` when the
  // inferior process is stopped.
  ProcessStoppedEvent wait_on_signal();

  pid_t pid() const;

private:
  pid_t _pid;
  ProcessState _state;
  bool _should_terminate_session_on_end;
};

} // namespace bdb

#endif // BDB_PROCESS_HPP_
