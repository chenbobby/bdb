#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <variant>

#include <editline/readline.h>

#include <libbdb/libbdb.hpp>
#include <libbdb/process.hpp>
#include <vector>

namespace {

std::vector<std::string> split(const std::string_view& str,
                               const char delimiter) {
  std::vector<std::string> out{};
  std::stringstream ss{std::string{str}};
  std::string s{};

  while (std::getline(ss, s, delimiter)) {
    out.push_back(s);
  }

  return out;
}

bool is_prefix(const std::string_view& str, const std::string_view& prefix) {
  if (str.size() < prefix.size()) {
    return false;
  }
  return std::equal(prefix.begin(), prefix.end(), str.begin());
}

void handle_command(std::unique_ptr<bdb::Process>& process,
                    const std::string_view& line) {
  // TODO: Refactor out line parsing from command handling.
  const auto args{split(line, ' ')};
  const auto command{args[0]};

  if (is_prefix(command, "continue")) {
    process->resume();
    auto process_stopped_event{process->wait_on_signal()};
    process_stopped_event.print(std::cerr);
    return;
  }

  std::cerr << "Unknown command: \"" << command << "\"" << std::endl;
}

struct CommandAttach {
  pid_t pid;
};

struct CommandLaunch {
  std::filesystem::path program_path;
  std::vector<std::string> args;
};

struct CommandMissing {};

using Command = std::variant<CommandAttach, CommandLaunch, CommandMissing>;

Command parse_input(int argc, const char** argv) {
  if (argc == 1) {
    return CommandMissing{};
  }

  if (argc == 3 && argv[1] == std::string_view{"-p"}) {
    const auto pid{std::atoi(argv[2])};
    return CommandAttach{pid_t{pid}};
  }

  const auto program_path{std::filesystem::path{argv[1]}};
  auto args{std::vector<std::string>(argc - 2)};
  for (auto i{2}; i < argc; ++i) {
    args.push_back(std::string{argv[i]});
  }
  return CommandLaunch{program_path, args};
}

void run_debug_session(std::unique_ptr<bdb::Process>& process) {
  char* line{nullptr};
  while ((line = readline("bdb> ")) != nullptr) {
    std::string line_str;

    if (line == std::string_view{""}) {
      free(line);
      if (history_length > 0) {
        line_str = std::string{history_list()[history_length - 1]->line};
      }
    } else {
      line_str = std::string{line};
      add_history(line);
      free(line);
    }

    if (!line_str.empty()) {
      handle_command(process, line_str);
    }
  }
}

} // namespace

int main(int argc, const char** argv) {
  const auto command{parse_input(argc, argv)};

  if (std::holds_alternative<CommandMissing>(command)) {
    std::cerr << "CLI command is missing" << std::endl;
    return EXIT_FAILURE;
  }

  if (std::holds_alternative<CommandAttach>(command)) {
    const auto command_attach{std::get<CommandAttach>(command)};
    auto process{bdb::Process::attach(command_attach.pid)};
    // TODO: Run debugging session.
    run_debug_session(process);
    exit(EXIT_SUCCESS);
  }

  assert(std::holds_alternative<CommandLaunch>(command));
  const auto command_launch{std::get<CommandLaunch>(command)};
  // TODO: Pass `args` into Process factory method.
  auto process{bdb::Process::launch(command_launch.program_path)};
  run_debug_session(process);
  exit(EXIT_SUCCESS);
}
