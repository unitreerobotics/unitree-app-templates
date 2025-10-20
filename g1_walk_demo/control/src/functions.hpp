#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include "unitree/robot/g1/arm/g1_arm_action_client.hpp"
#include "unitree/robot/g1/arm/g1_arm_action_error.hpp"
#include <unitree/robot/g1/loco/g1_loco_api.hpp>
#include <unitree/robot/g1/loco/g1_loco_client.hpp>

using namespace unitree::robot::g1;
using json = nlohmann::json;

/// Run a G1 arm action by ID.
int RunG1ArmAction(int action_id, std::string &out_result) {
  // Unitree DDS Initialization
  unitree::robot::ChannelFactory::Instance()->Init(0, "");

  auto client = std::make_shared<G1ArmActionClient>();
  client->Init();
  client->SetTimeout(2.f);

  if (action_id == 0) {
    std::string action_list_data;
    int32_t ret = client->GetActionList(action_list_data);
    if (ret != 0) {
      out_result = "Failed to get action list";
      return ret;
    }
    out_result = action_list_data;
    return 0;
  } else {
    int32_t ret = client->ExecuteAction(action_id);
    if (ret != 0) {
      switch (ret) {
      case UT_ROBOT_ARM_ACTION_ERR_ARMSDK:
        out_result = UT_ROBOT_ARM_ACTION_ERR_ARMSDK_DESC;
        break;
      case UT_ROBOT_ARM_ACTION_ERR_HOLDING:
        out_result = UT_ROBOT_ARM_ACTION_ERR_HOLDING_DESC;
        break;
      case UT_ROBOT_ARM_ACTION_ERR_INVALID_ACTION_ID:
        out_result = UT_ROBOT_ARM_ACTION_ERR_INVALID_ACTION_ID_DESC;
        break;
      case UT_ROBOT_ARM_ACTION_ERR_INVALID_FSM_ID:
        out_result = "Invalid FSM ID. Supported states {500, 501, 801}.";
        break;
      default:
        out_result =
            "Execute action failed, error code: " + std::to_string(ret);
        break;
      }
    } else {
      out_result = "Action executed successfully.";
    }
    return ret;
  }
}

std::vector<float> stringToFloatVector(const std::string &str) {
  std::vector<float> result;
  std::stringstream ss(str);
  float num;
  while (ss >> num) {
    result.push_back(num);
    ss.ignore();  // 忽略空格或分隔符
  }
  return result;
}

std::shared_ptr<LocoClient> initClient(const std::string &network_interface = "lo") {
  unitree::robot::ChannelFactory::Instance()->Init(0, network_interface);
  auto client = std::make_shared<LocoClient>();
  client->Init();
  client->SetTimeout(2.f);
  return client;
}

void executeCommand(const std::shared_ptr<LocoClient> &client,
                    const std::string &cmd,
                    const std::string &val = "") {
  if (cmd == "move") {
    auto param = stringToFloatVector(val);
    if (param.size() != 3) {
      throw std::runtime_error("move 需要三个参数 vx vy omega");
    }
    client->Move(param[0], param[1], param[2]);
    std::cout << "执行 move: vx=" << param[0]
              << " vy=" << param[1]
              << " omega=" << param[2] << std::endl;

  } else if (cmd == "wave_hand") {
    client->WaveHand();
    std::cout << "执行 wave_hand" << std::endl;

  } else if (cmd == "set_fsm_id") {
    int fsm_id = std::stoi(val);
    client->SetFsmId(fsm_id);
    std::cout << "执行 set_fsm_id: " << fsm_id << std::endl;

  } else if (cmd == "damp") {
    std::cout << "执行 damp 开始" << std::endl;
    auto code = client->Damp();
    std::cout << "执行 damp 结束 " << code <<std::endl;

  } else if (cmd == "stand") {
    client->StandUp();
    std::cout << "执行 stand" << std::endl;

  }else if (cmd == "start") {
    client->Start();
    std::cout << "执行 start" << std::endl;

  }else if (cmd == "squat") {
    client->Start();
    std::cout << "执行 squat" << std::endl;

  } else {
    throw std::runtime_error("未知命令: " + cmd);
  }
}