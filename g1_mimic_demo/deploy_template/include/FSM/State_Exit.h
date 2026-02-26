// Copyright (c) 2025, Unitree Robotics Co., Ltd.
// All rights reserved.

#pragma once

#include "FSMState.h"
#include "unitree/robot/g1/loco/g1_loco_api.hpp"
#include "unitree/robot/g1/loco/g1_loco_client.hpp"

class State_Exit : public FSMState
{
public:
    State_Exit(int state, std::string state_string = "Exit") 
    : FSMState(state, state_string) 
    {
        auto motor_mode = param::config["FSM"]["Passive"]["mode"];
        if(motor_mode.IsDefined())
        {
            auto values = motor_mode.as<std::vector<int>>();
            for(int i(0); i<values.size(); ++i)
            {
                lowcmd->msg_.motor_cmd()[i].mode() = values[i];
            }
        }
    } 

    void enter()
    {
        unitree::robot::g1::LocoClient client;
        // set gain
        static auto kd = param::config["FSM"]["Passive"]["kd"].as<std::vector<float>>();

        this->registered_checks.emplace_back(
            std::make_pair(
                [&]()-> bool { return keyboard->key() == "s"; },
                FSMStringMap.right.at("Mimic")
            )
        );
        
        client.Init();

        
        auto rpy = lowstate->msg_.imu_state().rpy();

        if(std::abs(rpy[0])>1.0f || std::abs(rpy[1])>0.8f)  // if the robot is in a bad state, switch to passive mode to avoid damage
        {
            client.SwitchToInternalCtrl(unitree::robot::g1::InternalFsmMode::PASSIVE);
        }
        else{
            client.SwitchToInternalCtrl(unitree::robot::g1::InternalFsmMode::WALKRUN);
        }
        
    }

    void run()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
};

REGISTER_FSM(State_Exit)
