#include <asm-generic/errno-base.h>
#include <cerrno>
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

} // namespace

TEST_CASE("Tracee::launch succeeds", "[tracee]") {
  const auto tracee{bdb::Tracee::launch("yes")};
  REQUIRE(pid_exists(tracee->pid()));
}

TEST_CASE("Tracee::launch throws when no such program", "[tracee]") {
  REQUIRE_THROWS_AS(bdb::Tracee::launch("no_such_program"), bdb::Error);
}
