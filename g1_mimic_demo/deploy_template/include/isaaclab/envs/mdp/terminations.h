#pragma once

#include "isaaclab/envs/manager_based_rl_env.h"
#include <array>
#include <cmath>
#include <Eigen/Geometry>

namespace isaaclab
{
namespace mdp
{

inline bool bad_orientation(ManagerBasedRLEnv* env, float limit_angle = 1.0)
{
    auto & asset = env->robot;
    auto & data = asset->data.projected_gravity_b;
    return std::fabs(std::acos(-data[2])) > limit_angle;
}

inline bool bad_orientation_rel(ManagerBasedRLEnv* env, Eigen::Quaternionf root_quat_rel, float limit_angle = 0.8)
{
    auto & asset = env->robot;
    // assume root_quat layout is [w, x, y, z]
    float qw = root_quat_rel.w();
    float qx = root_quat_rel.x();
    float qy = root_quat_rel.y();
    float qz = root_quat_rel.z();
    // rotate world gravity (0,0,-1) by quaternion: g_local = q * (0,0,-1) * q^*
    float gx = -2.0f * (qy * qw + qz * qx);
    float gy =  2.0f * (qx * qw - qy * qz);
    float gz = -1.0f + 2.0f * (qx * qx + qy * qy);
    auto & data = asset->data.projected_gravity_b;
    return std::fabs(gz - data[2]) > limit_angle;
}

} // namespace mdp
} // namespace isaaclab