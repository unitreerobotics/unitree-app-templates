#pragma once

#include "FSM/State_RLBase.h"
#include "unitree/robot/g1/loco/g1_loco_api.hpp"
#include "unitree/robot/g1/loco/g1_loco_client.hpp"

class State_Mimic : public FSMState
{
public:
    State_Mimic(int state_mode, std::string state_string);

    void enter();

    void run();
    
    void exit()
    {
        policy_thread_running = false;
        if (policy_thread.joinable()) {
            policy_thread.join();
        }
    }

    class MotionLoader_;

    static std::shared_ptr<MotionLoader_> motion; // for obs computation
private:
    std::unique_ptr<isaaclab::ManagerBasedRLEnv> env;
    std::shared_ptr<MotionLoader_> motion_; // for saving

    std::thread policy_thread;
    bool policy_thread_running = false;
    bool exit_mimic = false;
    std::array<float, 2> time_range_;
    unitree::robot::g1::LocoClient client;
};

class State_Mimic::MotionLoader_
{
public:
    MotionLoader_(std::string motion_file, float fps)
    : dt(1.0f / fps)
    {
        auto data = isaaclab::load_csv(motion_file);
        
        num_frames = data.size();
        duration = num_frames * dt;
        
        for(int i(0); i < num_frames; ++i)
        {
            root_positions.push_back(Eigen::VectorXf::Map(data[i].data(), 3));
            root_quaternions.push_back(Eigen::Quaternionf(data[i][6],data[i][3], data[i][4], data[i][5]));
            dof_positions.push_back(Eigen::VectorXf::Map(data[i].data() + 7, data[i].size() - 7));
        }

        update(0.0f);
    }

    void update(float time) 
    {
        current_time_ = std::clamp(time, 0.0f, duration);
        float phase = std::clamp(current_time_ / duration, 0.0f, 1.0f);
        index_0_ = std::round(phase * (num_frames - 1));
        index_1_ = std::min(index_0_ + 1, num_frames - 1);
        blend_ = std::round((current_time_ - index_0_ * dt) / dt * 1e5f) / 1e5f;
    }

    void reset(const isaaclab::ArticulationData & data, float t = 0.0f)
    {
        update(t);
        auto init_to_anchor = isaaclab::yawQuaternion(this->root_quaternion()).toRotationMatrix();
        auto world_to_anchor = isaaclab::yawQuaternion(data.root_quat_w).toRotationMatrix();
        world_to_init_ = world_to_anchor * init_to_anchor.transpose();
    }

    Eigen::VectorXf joint_pos() {
        return _interp_dof_position(current_time_);
    }

    Eigen::VectorXf root_position() {
        return root_positions[index_0_] * (1 - blend_) + root_positions[index_1_] * blend_;
    }

    Eigen::VectorXf joint_vel() {
        if (num_frames < 2) {
            return Eigen::VectorXf::Zero(dof_positions.empty() ? 0 : dof_positions.front().size());
        }

        const float prev_t = std::max(0.0f, current_time_ - dt);
        const float next_t = std::min(duration, current_time_ + dt);

        if (next_t <= prev_t) {
            return Eigen::VectorXf::Zero(dof_positions.front().size());
        }

        return (_interp_dof_position(next_t) - _interp_dof_position(prev_t)) / (next_t - prev_t);
    }

    Eigen::Quaternionf root_quaternion() {
        return root_quaternions[index_0_].slerp(blend_, root_quaternions[index_1_]);
    }

    float dt;
    int num_frames;
    float duration;

    std::vector<Eigen::VectorXf> root_positions;
    std::vector<Eigen::Quaternionf> root_quaternions;
    std::vector<Eigen::VectorXf> dof_positions;

    Eigen::Matrix3f world_to_init_;
private:
    int index_0_;
    int index_1_;
    float blend_;
    float current_time_ = 0.0f;

    Eigen::VectorXf _interp_dof_position(float time) const
    {
        if (dof_positions.empty()) {
            return Eigen::VectorXf();
        }

        const float clamped_time = std::clamp(time, 0.0f, duration);
        const float phase = std::clamp(clamped_time / duration, 0.0f, 1.0f);
        const int index_0 = std::round(phase * (num_frames - 1));
        const int index_1 = std::min(index_0 + 1, num_frames - 1);
        const float blend = std::round((clamped_time - index_0 * dt) / dt * 1e5f) / 1e5f;
        return dof_positions[index_0] * (1 - blend) + dof_positions[index_1] * blend;
    }
};

REGISTER_FSM(State_Mimic)
