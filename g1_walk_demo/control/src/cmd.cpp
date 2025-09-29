#include "functions.hpp"

static int runArmAction(int id) {
  std::string msg;
  int ret = RunG1ArmAction(id, msg);
  std::cout << (ret == 0 ? "[OK] " : "[ERR] ") << msg << std::endl;
  return ret;
}

static int runLocoCmd(const std::string &cmd, const std::string &val) {
  auto client = initClient("");
  executeCommand(client, cmd, val);
  std::cout << "[OK] loco_command: " << cmd << " " << val << std::endl;
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "用法:\n"
              << "  " << argv[0] << " arm_actions\n"
              << "  " << argv[0] << " arm_action <id>\n"
              << "  " << argv[0] << " loco_command <cmd> [val]\n"
              << "  " << argv[0] << " move <vx> <vy> <omega>\n";
    return 1;
  }

  std::string cmd = argv[1];
  try {
    if (cmd == "arm_actions") {
      return runArmAction(0);
    } else if (cmd == "arm_action" && argc >= 3) {
      return runArmAction(std::stoi(argv[2]));
    } else if (cmd == "loco_command" && argc >= 3) {
      return runLocoCmd(argv[2], argc >= 4 ? argv[3] : "");
    } else if (cmd == "move" && argc >= 5) {
      std::ostringstream oss;
      oss << argv[2] << " " << argv[3] << " " << argv[4];
      return runLocoCmd("move", oss.str());
    } else {
      std::cerr << "参数错误或未知命令\n";
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "[异常] " << e.what() << std::endl;
    return 1;
  }
}