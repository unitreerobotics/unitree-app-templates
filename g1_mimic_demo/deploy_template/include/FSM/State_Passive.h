// Copyright (c) 2025, Unitree Robotics Co., Ltd.
// All rights reserved.

#pragma once

#include "FSMState.h"
#include "unitree/robot/g1/loco/g1_loco_api.hpp"
#include "unitree/robot/g1/loco/g1_loco_client.hpp"

class State_Passive : public FSMState
{
public:
    State_Passive(int state, std::string state_string = "Passive") 
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

        loco_client_.Init();
    } 

    void enter()
    {
        static bool first_enter = true;
        // set gain
        static auto kd = param::config["FSM"]["Passive"]["kd"].as<std::vector<float>>();
        for(int i(0); i < kd.size(); ++i)
        {
            auto & motor = lowcmd->msg_.motor_cmd()[i];
            motor.kp() = 0;
            motor.kd() = kd[i];
            motor.dq() = 0;
            motor.tau() = 0;
        }

        if (!transition_registered_) {
            this->registered_checks.emplace_back(
                std::make_pair(
                    [&]()-> bool { return keyboard->key() == "s"; },
                    FSMStringMap.right.at("Mimic")
                )
            );
            transition_registered_ = true;
        }
        if(first_enter)
        {
            first_enter = false;
        }
        else
        {
            loco_client_.SwitchToInternalCtrl(unitree::robot::g1::InternalFsmMode::PASSIVE);
        }
        lowcmd->unlockAndPublish();
    }

    void run()
    {
        lowstate->update();
        if(keyboard) keyboard->update();
        for(int i(0); i < lowcmd->msg_.motor_cmd().size(); ++i)
        {
            lowcmd->msg_.motor_cmd()[i].q() = lowstate->msg_.motor_state()[i].q();
        }
    }

private:
    unitree::robot::g1::LocoClient loco_client_;
    bool transition_registered_ = false;
};

REGISTER_FSM(State_Passive)
