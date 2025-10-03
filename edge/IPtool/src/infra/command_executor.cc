#include "infra/command_executor.hpp"

#include <array>
#include <cerrno>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include <sys/wait.h>

#include "common/logging.hpp"

namespace iptool::infra {

namespace {
std::string EscapeArgument(std::string_view arg) {
  std::string escaped;
  escaped.reserve(arg.size() + 2);
  escaped.push_back('\'');
  for (const char ch : arg) {
    if (ch == '\'') {
      escaped.append("'\\''");
    } else {
      escaped.push_back(ch);
    }
  }
  escaped.push_back('\'');
  return escaped;
}
}

CommandResult PosixCommandExecutor::Execute(const Command& command) {
  const auto cmd_line = BuildCommandLine(command);
  LogInfoFormat("Executing command: {}", cmd_line);

  FILE* pipe = popen(cmd_line.c_str(), "r");
  if (!pipe) {
    throw std::system_error(errno, std::generic_category(),
                            "popen failed for command: " + cmd_line);
  }

  std::array<char, 512> buffer{};
  std::string output;
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
    output.append(buffer.data());
  }

  const int status = pclose(pipe);
  int exit_code = status;
  if (WIFEXITED(status)) {
    exit_code = WEXITSTATUS(status);
  }

  LogInfoFormat("Command exit_code={} output={}", exit_code,
                 output.empty() ? "<empty>" : output);
  return {exit_code, output};
}

std::string PosixCommandExecutor::BuildCommandLine(const Command& command) {
  std::ostringstream oss;
  oss << command.executable;
  for (const auto& arg : command.arguments) {
    oss << ' ' << EscapeArgument(arg);
  }
  return oss.str();
}

}  // namespace iptool::infra
