#include <cstdlib>
#include <cstring>
#include <libbdb/tracee.hpp>

#include <iostream> // std::perror, std::cerr, std::endl
#include <memory>   // std::unique_ptr, std::make_unique
#include <ostream>
#include <sys/ptrace.h> // ptrace, PTRACE_CONT
#include <sys/wait.h>   // waitpid

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

  std::cerr
      << "TraceeStoppedEvent constructor received unexpected wait_status \""
      << wait_status << "\"" << std::endl;
  exit(EXIT_FAILURE);
}

void TraceeStoppedEvent::print(std::ostream& out) {
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

Tracee::~Tracee() {
  if (this->_pid == 0) {
    return;
  }

  int wait_status;
  int wait_options{0};
  if (this->_state == TraceeState::RUNNING) {
    std::cerr << "Stopping inferior tracee..." << std::endl;
    kill(this->_pid, SIGSTOP);
    waitpid(this->_pid, &wait_status, wait_options);
  }
  std::cerr << "Inferier tracee is stopped." << std::endl;

  std::cerr << "Detaching from inferior tracee..." << std::endl;
  ptrace(PTRACE_DETACH, this->_pid, nullptr, nullptr);
  kill(this->_pid, SIGCONT);

  if (this->_should_terminate_session_on_end) {
    std::cerr << "Terminating inferior tracee..." << std::endl;
    kill(this->_pid, SIGKILL);
    waitpid(this->_pid, &wait_status, wait_options);
  }
}

std::unique_ptr<Tracee> Tracee::launch(const std::filesystem::path& path) {
  pid_t pid;
  if ((pid = fork()) == 0) {
    // Newly forked process.
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr)) {
      std::perror("failed to attach to the newly forked process");
      exit(EXIT_FAILURE);
    }

    if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
      std::perror("failed to exec the newly forked process");
      exit(EXIT_FAILURE);
    }
    // Unreachable if newly forked tracee has successfully exec'ed.
  }

  return std::make_unique<Tracee>(pid, true);
}

// Constructs a `Tracee` by attaching to an existing process with PID `pid`.
std::unique_ptr<Tracee> Tracee::attach(const pid_t& pid) {
  if (pid == 0) {
    std::cerr << "invalid pid \"" << pid << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    std::perror("failed to attach to existing process");
    exit(EXIT_FAILURE);
  }

  return std::make_unique<Tracee>(pid, true);
}

void Tracee::resume() {
  if (ptrace(PTRACE_CONT, this->_pid, nullptr, nullptr) < 0) {
    std::perror("failed to continue tracing");
    exit(EXIT_FAILURE);
  }
  this->_state = TraceeState::RUNNING;
}

TraceeStoppedEvent Tracee::wait_on_signal() {
  int wait_status;
  int wait_options{0};
  if (waitpid(this->_pid, &wait_status, wait_options) < 0) {
    std::perror("failed to wait on tracee");
    exit(EXIT_FAILURE);
  }

  auto tracee_stopped_event{TraceeStoppedEvent{this->_pid, wait_status}};
  this->_state = tracee_stopped_event.tracee_state();
  return tracee_stopped_event;
}

pid_t Tracee::pid() const { return this->_pid; }

} // namespace bdb