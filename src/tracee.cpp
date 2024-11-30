#include <cstdlib>
#include <cstring>
#include <iostream> // std::perror, std::cerr, std::endl
#include <memory>   // std::unique_ptr, std::make_unique
#include <ostream>
#include <string>
#include <sys/ptrace.h> // ptrace, PTRACE_CONT
#include <sys/wait.h>   // waitpid

#include <libbdb/error.hpp>
#include <libbdb/pipe.hpp>
#include <libbdb/tracee.hpp>

namespace bdb {

TraceeStoppedEvent::TraceeStoppedEvent(const pid_t& pid, const int wait_status)
    : _pid{pid} {
  if (WIFEXITED(wait_status)) {
    this->_tracee_state = TraceeState::EXITED;
    this->_info = WEXITSTATUS(wait_status);
    return;
  }

  if (WIFSTOPPED(wait_status)) {
    this->_tracee_state = TraceeState::STOPPED;
    this->_info = WSTOPSIG(wait_status);
    return;
  }

  if (WIFSIGNALED(wait_status)) {
    this->_tracee_state = TraceeState::TERMINATED;
    this->_info = WTERMSIG(wait_status);
    return;
  }

  throw Error{
      "TraceeStoppedEvent constructor received unexpected wait_status \"" +
      std::to_string(wait_status) + "\""};
}

void TraceeStoppedEvent::print(std::ostream& out) noexcept {
  out << "Tracee (" << this->_pid << ") ";

  switch (this->_tracee_state) {
  case TraceeState::RUNNING:
    out << "exited with exit code \"" << sigabbrev_np(this->_info) << "\"";
  case TraceeState::STOPPED:
    out << "stopped with signal \"" << sigabbrev_np(this->_info) << "\"";
    break;
  case TraceeState::TERMINATED:
    out << "terminated with signal \"" << sigabbrev_np(this->_info) << "\"";
    break;
  case TraceeState::EXITED:
    break;
  }

  out << std::endl;
}

Tracee::Tracee() noexcept
    : _pid{0}, _state{TraceeState::STOPPED}, _should_terminate_session_on_end{
                                                 false} {}

Tracee::Tracee(const pid_t& pid,
               const bool should_terminate_session_on_end) noexcept
    : _pid{pid}, _state{TraceeState::STOPPED},
      _should_terminate_session_on_end{should_terminate_session_on_end} {}

Tracee::~Tracee() noexcept {
  if (this->_pid == 0) {
    return;
  }

  int wait_status;
  int wait_options{0};
  if (this->_state == TraceeState::RUNNING) {
    std::cerr << "Stopping pid (" << this->_pid << ")..." << std::endl;
    kill(this->_pid, SIGSTOP);
    waitpid(this->_pid, &wait_status, wait_options);
  }
  std::cerr << "Stopped pid (" << this->_pid << ")." << std::endl;

  std::cerr << "Detaching from pid (" << this->_pid << ")..." << std::endl;
  ptrace(PTRACE_DETACH, this->_pid, nullptr, nullptr);
  std::cerr << "Detached from pid (" << this->_pid << ")." << std::endl;

  kill(this->_pid, SIGCONT);

  if (this->_should_terminate_session_on_end) {
    std::cerr << "Terminating pid (" << this->_pid << ")..." << std::endl;
    kill(this->_pid, SIGKILL);
    waitpid(this->_pid, &wait_status, wait_options);
    std::cerr << "Terminated pid (" << this->_pid << ")." << std::endl;
  }
}

std::unique_ptr<Tracee> Tracee::launch(const std::filesystem::path& path) {
  auto channel{Pipe{true}};

  pid_t pid;
  if ((pid = fork()) == 0) {
    // Newly forked process.
    channel.close_receiver();
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
      channel.send(
          message_with_errno("failed to trace the newly forked process"));
      exit(EXIT_FAILURE);
    }

    if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
      channel.send(
          message_with_errno("failed to exec the newly forked process"));
      exit(EXIT_FAILURE);
    }
    // Unreachable if newly forked process has successfully exec'ed.
  }
  channel.close_sender();

  const auto data{channel.receive()};
  if (data.size() > 0) {
    waitpid(pid, nullptr, 0);
    throw Error{data};
  }

  auto tracee{std::make_unique<Tracee>(pid, true)};
  tracee->wait_on_signal();
  return tracee;
}

// Constructs a `Tracee` by attaching to an existing process with PID `pid`.
std::unique_ptr<Tracee> Tracee::attach(const pid_t& pid) {
  if (pid == 0) {
    throw Error{"invalid pid \"" + std::to_string(pid) + "\""};
  }

  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    throw Error::with_errno("failed to attach to pid \"" + std::to_string(pid) +
                            "\"");
  }

  return std::make_unique<Tracee>(pid, true);
}

void Tracee::resume() {
  if (ptrace(PTRACE_CONT, this->_pid, nullptr, nullptr) < 0) {
    throw Error::with_errno("failed to resume pid (" +
                            std::to_string(this->_pid) + ")");
  }
  this->_state = TraceeState::RUNNING;
}

TraceeStoppedEvent Tracee::wait_on_signal() {
  int wait_status;
  int wait_options{0};
  if (waitpid(this->_pid, &wait_status, wait_options) < 0) {
    throw Error::with_errno("failed to wait on pid (" +
                            std::to_string(this->_pid) + ")");
  }

  auto tracee_stopped_event{TraceeStoppedEvent{this->_pid, wait_status}};
  this->_state = tracee_stopped_event.tracee_state();
  return tracee_stopped_event;
}

pid_t Tracee::pid() const noexcept { return this->_pid; }

} // namespace bdb
