#include <asm-generic/errno-base.h>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <stdexcept>
#include <sys/types.h>

#include <catch2/catch_test_macros.hpp>
#include <libbdb/error.hpp>
#include <libbdb/tracee.hpp>

namespace {

// Returns true if `pid` exists on the system. Otherwise returns false.
bool pid_exists(const pid_t pid) {
  if (kill(pid, 0) == -1) {
    return false;
  }
  if (errno == ESRCH) {
    return false;
  }
  return true;
}

enum class ProcessStatus {
  STOPPED_BY_TRACE,
  UNKNOWN,
};

// Returns the status of a pid.
ProcessStatus process_status(const pid_t pid) {
  auto proc_stat{std::ifstream{"/proc/" + std::to_string(pid) + "/stat"}};
  auto data{std::string()};
  std::getline(proc_stat, data);
  auto index_of_last_closing_paren{data.rfind(")")};
  auto index_of_status{index_of_last_closing_paren + 2};
  switch (data[index_of_status]) {
  case 't':
    return ProcessStatus::STOPPED_BY_TRACE;
  default:
    return ProcessStatus::UNKNOWN;
  }
}

} // namespace

TEST_CASE("Tracee::launch succeeds", "[Tracee]") {
  const auto tracee{bdb::Tracee::launch("yes")};
  REQUIRE(pid_exists(tracee->pid()));
}

TEST_CASE("Tracee::launch throws when no such program", "[Tracee]") {
  REQUIRE_THROWS_AS(bdb::Tracee::launch("no_such_program"), bdb::Error);
}

TEST_CASE("Tracee::attach succeeds", "[Tracee]") {
  const auto pid{fork()};
  if (pid == 0) {
    if (execlp("yes", "yes", nullptr) < 0) {
      std::cerr << "failed to exec new process" << std::endl;
    }
  }

  const auto tracee{bdb::Tracee::attach(pid)};
  REQUIRE(process_status(pid) == ProcessStatus::STOPPED_BY_TRACE);
}
