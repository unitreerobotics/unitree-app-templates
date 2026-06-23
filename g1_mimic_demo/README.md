# G1 Mimic Demo UniStore Deployment Template

## Purpose

`g1_mimic_demo` is a deployment template for Unitree G1 / UniStore. Its goal is not to replace the official motion controller. Instead, it keeps the official controller available, temporarily switches to a user-defined controller, and then hands control back to the official controller when the motion finishes, fails, or becomes unsafe.

The current example FSM state is `Mimic`, which runs a motion imitation policy. You can treat this project as a practical skeleton: keep the UniStore packaging, DDS communication, FSM switching, and fallback logic, then replace the policy, motion data, or custom control state.

## Control Flow

```text
UniStore UI / API
  -> app/g1/cmd_server.py
  -> app/g1/run_g1_ctrl.sh
  -> deploy_template/out/bin/g1_ctrl
  -> FSM: Passive -> Mimic -> Exit / Passive
  -> rt/user_lowcmd
```

Key points:

- UniStore starts `app/g1/cmd_server.py`, which manages a long-running controller process.
- The default Start action sends `s` to the controller process, moving the template FSM from `Passive` to `Mimic`.
- `State_Mimic` calls `LocoClient::SwitchToUserCtrl()` and keeps publishing user motor commands only after the handoff is verified.
- User motor commands are published to `rt/user_lowcmd` by default. See `deploy_template/include/unitree/dds_wrapper/robots/g1/g1_pub.h`.
- When the motion ends, the policy thread exits, the orientation becomes unsafe, or an abnormal FSM state is detected, the FSM returns to `Exit` or `Passive` and uses the official `LocoClient` path to switch back to internal control.

## Directory Layout

```text
g1_mimic_demo/
  app/g1/                    # UniStore runtime entrypoint, API server, startup script
  deploy_template/           # C++ controller source, config, models, third-party libs, build script
  deploy_template/config/    # FSM, policy, and motion configuration
  deploy_template/src/       # User control state implementation, currently State_Mimic.cpp
  deploy_template/include/   # FSM, DDS wrapper, Isaac Lab-style interfaces
  Dockerfile.arm64-builder   # ARM64 Ubuntu 20.04 builder image
  docker_build_arm64.sh      # Recommended build script; syncs artifacts to app/g1/app_out
  metadata.yaml              # UniStore application metadata
```

## What To Change For Your Own App

### 1. Replace the Motion and Policy

Most users start by editing the `FSM.Mimic` section in `deploy_template/config/config.yaml`:

```yaml
Mimic:
  fps: 50
  motion_file: config/policy/mimic/unistore/params/HOUKONGFAN_50hz.csv
  policy_dir: config/policy/mimic/unistore/
  policy_file: config/policy/mimic/unistore/exported/policy.mnn
  time_start: 2.0
  time_end: 3.0
```

Fields:

- `motion_file`: reference motion CSV.
- `policy_dir`: policy deployment directory, usually containing `params/deploy.yaml`.
- `policy_file`: MNN policy model.
- `fps`: frame rate of the motion file.
- `time_start` / `time_end`: motion clip range. Use a short range first when testing on hardware.

## ONNX To MNN

If your policy model is currently in `.onnx` format, install the MNN conversion tools first, then convert the model:

```bash
python3 -m pip install -U MNN
python3 -m pip install -U onnx
MNNConvert -f ONNX --modelFile model.onnx --MNNModel model.mnn --bizCode MNN
```

Here, `model.onnx` is the input model and `model.mnn` is the output model name. Replace them with your actual file names as needed.

If `MNNConvert` is not found after installation, you can also try invoking it through the Python module:

```bash
python3 -m MNN.tools.mnnconvert -f ONNX --modelFile model.onnx --MNNModel model.mnn --bizCode MNN
```

### 2. Change Policy Observations or Actions

If your model input or output differs from the example, update these files together:

- `deploy_template/config/policy/.../params/deploy.yaml`
- registered observations in `deploy_template/src/State_Mimic.cpp`
- `MotionLoader_` in `deploy_template/include/State_Mimic.h`

For the first hardware test, keep joint order, action dimension, and `joint_ids_map` as close to the example as possible. Once the template runs reliably, widen the change.

### 3. Add Your Own Control State

The safest path is to copy `State_Mimic.cpp` / `State_Mimic.h` and rename the class, for example to `State_MyController`.

1. Add `REGISTER_FSM(State_MyController)` at the end of the header.
2. Enable the state in `FSM._` inside `deploy_template/config/config.yaml`.
3. Add a config section for the new state.
4. Include the new state header from `main.cpp`.
5. Confirm that `Passive`, `Exit`, or your chosen trigger state can transition into the new state.

If you rename `Mimic`, search the whole project for `Mimic`. Some transition targets and publish guards are tied to that name and should be updated together.

## Safety And Fallback Logic

The template includes several protection layers, but it does not replace careful hardware testing:

- `Mimic` calls `SwitchToUserCtrl()` and checks after 0.1 s that the official FSM ID is `1000`.
- If user-control handoff fails, the policy thread exits and the FSM falls back.
- If `Mimic` reaches `time_end` or the policy thread stops, the FSM switches to `Exit`.
- If orientation error is too large, the FSM switches to `Passive`.
- `Exit` returns to official `WALKRUN` or `PASSIVE` according to IMU attitude.

Before running on hardware:

- Start with a 1 to 3 second `time_start` / `time_end` clip.
- Test with a stand, suspension, or another safe setup first.
- Check that `motion_file`, `policy_file`, and `deploy.yaml` use matching joint order and dimensions.
- Make sure the UniStore app has the `motor` permission.
- Make sure no other process is publishing conflicting low-level motor commands.

## Build

### Recommended: ARM64 Docker Build

The root directory provides:

- `Dockerfile.arm64-builder`: builds the toolchain image.
- `docker_build_arm64.sh`: bind-mounts this repository into `/workspace` and builds in place.

Usage:

```bash
chmod +x docker_build_arm64.sh
./docker_build_arm64.sh
```

The script will:

1. Build an Ubuntu 20.04 ARM64 builder image.
2. Install and build `unitree_sdk2`.
3. Run `deploy_template/install.sh`.
4. Sync artifacts to `app/g1/app_out`.

To change the output directory:

```bash
APP_OUTPUT_DIR=/your/output/path ./docker_build_arm64.sh
```

To pin the `unitree_sdk2` branch:

```bash
docker build \
  --platform linux/arm64 \
  --build-arg UNITREE_SDK2_REF=main \
  -f Dockerfile.arm64-builder \
  -t g1-unistore-builder:ubuntu20.04-arm64 .
```

### Manual Dependency Installation

If you do not use Docker, install dependencies in an Ubuntu 20.04 ARM64 environment:

```bash
sudo apt install -y libyaml-cpp-dev libboost-all-dev libeigen3-dev libspdlog-dev libfmt-dev
git clone git@github.com:unitreerobotics/unitree_sdk2.git
cd unitree_sdk2
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=OFF
sudo make install
```

Then build the controller:

```bash
cd deploy_template
./install.sh
```

After build, make sure `app/g1/app_out` contains:

```text
bin/g1_ctrl
config/
lib/
```

## Local Simulation Testing

For simulation testing, you can change the default `LowCmd` topic in `deploy_template/include/unitree/dds_wrapper/robots/g1/g1_pub.h` to `rt/lowcmd`, then run:

```bash
cd deploy_template
./run_g1_ctrl.sh
```

The default script runs `./g1_ctrl -n lo`. If your DDS network interface is not `lo`, edit `deploy_template/run_g1_ctrl.sh`.

The UniStore package uses:

```bash
app/g1/run_g1_ctrl.sh
```

## UniStore API Entrypoint

`app/g1/config.py` defaults to:

```python
EXECUTABLE_BIN = "/workspace/app/g1/run_g1_ctrl.sh"
REALTIME=True

OPTIONS = [
    {"id": 1, "name": "Start", "cmd": "s", "args": []},
]
```

If you need a manual stop action, add a Stop option, for example one that sends `o`, and handle that key in the C++ control state.

## Application Metadata

Fill in `metadata.yaml`. Example:

```yaml
version: v0.1.0
base_image: image/sdk2_cpp_py:latest
permissions:
  - camera
  - motor
build_env:
  - apt update && apt install -y python3 pip && pip install fastapi uvicorn -i https://pypi.tuna.tsinghua.edu.cn/simple

startup_commands:
  - python3 /workspace/app/g1/cmd_server.py
change_log:
  - Initial version.
```

If your app does not use the camera, remove `camera` according to UniStore requirements. Keep `motor` for motor-control apps.

## Package

After building and confirming that `app/g1/app_out` exists, package from the `g1_mimic_demo` directory:

```bash
tar -czvf your_app_name.tar.gz ./app
```

Upload `your_app_name.tar.gz` to UniStore.

## FAQ

### Start does not enter user control

Check the `Current FSM ID` in the log. The template checks that the official FSM ID becomes `1000` after entering user control. If it does not, the current official controller state may not allow user-control handoff. Currently, entering user control is only supported from WALKRUN mode, or `SwitchToUserCtrl()` may have failed.

### The policy starts but the motion is wrong

Check joint order, action dimension, default joint position, and history settings across `motion_file`, `policy_file`, and `deploy.yaml`.

### I want to test with a local topic

UniStore uses `rt/user_lowcmd` by default. Apps uploaded to the official UniStore platform must use this topic. For local testing, edit the default `LowCmd` topic in `deploy_template/include/unitree/dds_wrapper/robots/g1/g1_pub.h`.

### The app breaks after renaming the FSM state

Search for the old state name, such as `Mimic`. Check `config.yaml`, transition targets in `Passive` / `Exit`, state registration, `main.cpp` includes, and any publish protection logic.
