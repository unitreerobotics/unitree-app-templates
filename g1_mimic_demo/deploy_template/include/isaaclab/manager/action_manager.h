// Copyright (c) 2025, Unitree Robotics Co., Ltd.
// All rights reserved.

#pragma once

#include "isaaclab/envs/manager_based_rl_env.h"
#include "isaaclab/manager/manager_term_cfg.h"
#include <numeric>
#include <atomic>

namespace isaaclab
{

class ActionTerm 
{
public:
    ActionTerm(YAML::Node cfg, ManagerBasedRLEnv* env): cfg(cfg), env(env) {}

    virtual int action_dim() = 0;
    virtual std::vector<float> raw_actions() = 0;
    virtual std::vector<float> processed_actions() = 0;
    virtual void process_actions(std::vector<float> actions) = 0;
    virtual void reset(){};

protected:
    YAML::Node cfg;
    ManagerBasedRLEnv* env;
};

inline std::map<std::string, std::function<std::unique_ptr<ActionTerm>(YAML::Node, ManagerBasedRLEnv*)>>& actions_map() {
    static std::map<std::string, std::function<std::unique_ptr<ActionTerm>(YAML::Node, ManagerBasedRLEnv*)>> instance;
    return instance;
}

#define REGISTER_ACTION(name) \
    inline struct name##_registrar { \
        name##_registrar() { \
            actions_map()[#name] = [](YAML::Node cfg, ManagerBasedRLEnv* env) { \
                return std::make_unique<name>(cfg, env); \
            }; \
        } \
    } name##_registrar_instance;

class ActionManager
{
public:
    ActionManager(YAML::Node cfg, ManagerBasedRLEnv* env)
    : cfg(cfg), env(env)
    {
        _prepare_terms();

        int dim = total_action_dim();
        _action.resize(dim, 0.0f);

        _action_buffer[0].resize(dim, 0.0f);
        _action_buffer[1].resize(dim, 0.0f);
    }

    void reset()
    {
        std::fill(_action.begin(), _action.end(), 0.0f);
        std::fill(_action_buffer[0].begin(), _action_buffer[0].end(), 0.0f);
        std::fill(_action_buffer[1].begin(), _action_buffer[1].end(), 0.0f);

        for(auto & term : _terms)
        {
            term->reset();
        }
    }

    std::vector<float> action()
    {
        return _action;
    }


    std::vector<float> processed_actions()
    {
        std::vector<float> actions;
        for(auto & term : _terms)
        {
            auto term_action = term->processed_actions();
            actions.insert(actions.end(), term_action.begin(), term_action.end());
        }
        return actions;
    }


    void process_action(const std::vector<float>& action)
    {
        _action = action;

        int idx = 0;
        for(auto & term : _terms)
        {
            auto term_action = std::vector<float>(
                action.begin() + idx,
                action.begin() + idx + term->action_dim()
            );
            term->process_actions(term_action);
            idx += term->action_dim();
        }


        std::vector<float> full_action;
        full_action.reserve(total_action_dim());

        for(auto & term : _terms)
        {
            auto term_action = term->processed_actions();
            full_action.insert(full_action.end(), term_action.begin(), term_action.end());
        }


        int next = 1 - _active_idx.load(std::memory_order_relaxed);
        _action_buffer[next] = std::move(full_action);
        _active_idx.store(next, std::memory_order_release);
    }


    const std::vector<float>& latest_action()
    {
        return _action_buffer[_active_idx.load(std::memory_order_acquire)];
    }

    int total_action_dim()
    {
        auto dims = action_dim();
        return std::accumulate(dims.begin(), dims.end(), 0);
    }

    std::vector<int> action_dim()
    {
        std::vector<int> dims;
        for (auto & term : _terms)
        {
            dims.push_back(term->action_dim());
        }
        return dims;
    }

    YAML::Node cfg;
    ManagerBasedRLEnv* env;

private:
    void _prepare_terms()
    {
        for(auto it = this->cfg.begin(); it != this->cfg.end(); ++it)
        {
            std::string action_name = it->first.as<std::string>();
            if(actions_map().find(action_name) == actions_map().end())
            {
                throw std::runtime_error("Action term '" + action_name + "' is not registered.");
            }

            auto term = actions_map()[action_name](it->second, env);
            _terms.push_back(std::move(term));
        }
    }

private:
    std::vector<float> _action;
    std::vector<std::unique_ptr<ActionTerm>> _terms;


    std::vector<float> _action_buffer[2];
    std::atomic<int> _active_idx{0};
};

}; 