#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace iptool::infra {

struct Command {
  std::string executable;
  std::vector<std::string> arguments;
};

struct CommandResult {
  int exit_code = -1;
  std::string output;
};

class CommandExecutor {
 public:
  virtual ~CommandExecutor() = default;

  virtual CommandResult Execute(const Command& command) = 0;
};

class PosixCommandExecutor final : public CommandExecutor {
 public:
  CommandResult Execute(const Command& command) override;

 private:
  static std::string BuildCommandLine(const Command& command);
};

}  // namespace iptool::infra
