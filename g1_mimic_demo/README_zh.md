# G1 Mimic Demo UniStore 部署模板

## 项目定位

`g1_mimic_demo` 是一个面向 Unitree G1 / UniStore 的部署代码模板。它的目标不是替换官方运控，而是在官方运控仍然存在的前提下，短时间接管到用户自定义运控，执行完成、异常或姿态不安全时，再把控制权交回官方运控。

当前示例状态机为 `Mimic`，用于运行一个动作模仿策略。你可以把它当成一个可改造的骨架：保留 UniStore 打包、DDS 通信、状态切换和安全回退逻辑，只替换自己的模型、动作数据或控制状态。

## 核心控制流

```text
UniStore UI / API
  -> app/g1/cmd_server.py
  -> app/g1/run_g1_ctrl.sh
  -> deploy_template/out/bin/g1_ctrl
  -> FSM: Passive -> Mimic -> Exit / Passive
  -> rt/user_lowcmd
```

关键点如下：

- UniStore 侧通过 `app/g1/cmd_server.py` 启动一个常驻进程。
- 默认 Start 按钮向控制进程输入 `s`，触发模板 FSM 从 `Passive` 进入 `Mimic`。
- `State_Mimic` 进入后调用 `LocoClient::SwitchToUserCtrl()`，确认官方 FSM 切到用户控制模式后，才持续发布用户电机指令。
- 用户电机指令默认发布到 `rt/user_lowcmd`，见 `deploy_template/include/unitree/dds_wrapper/robots/g1/g1_pub.h`。
- 动作结束、策略线程退出、姿态异常或检测状态机异常后，FSM 会进入 `Exit` 或 `Passive`，并通过官方 `LocoClient` 切回官方内部控制。

## 目录说明

```text
g1_mimic_demo/
  app/g1/                    # UniStore 运行入口，包含 API 服务和启动脚本
  deploy_template/           # C++ 控制器源码、配置、模型、第三方库和构建脚本
  deploy_template/config/    # FSM、policy、motion 等运行配置
  deploy_template/src/       # 用户控制状态实现，示例为 State_Mimic.cpp
  deploy_template/include/   # FSM、DDS wrapper、Isaac Lab 风格接口等
  Dockerfile.arm64-builder   # ARM64 Ubuntu 20.04 构建环境
  docker_build_arm64.sh      # 推荐构建脚本，构建后同步产物到 app/g1/app_out
  metadata.yaml              # UniStore 应用元数据
```

## 二开时最常改的地方

### 1. 替换动作和策略

主要修改 `deploy_template/config/config.yaml` 中的 `FSM.Mimic` 配置：

```yaml
Mimic:
  fps: 50
  motion_file: config/policy/mimic/unistore/params/HOUKONGFAN_50hz.csv
  policy_dir: config/policy/mimic/unistore/
  policy_file: config/policy/mimic/unistore/exported/policy.mnn
  time_start: 2.0
  time_end: 3.0
```

字段含义：

- `motion_file`：动作参考 CSV。
- `policy_dir`：策略部署配置目录，通常包含 `params/deploy.yaml`。
- `policy_file`：MNN 策略模型。
- `fps`：动作文件帧率，需要与 CSV 数据一致。
- `time_start` / `time_end`：截取动作片段，适合先用短片段验证安全性。

## ONNX 转 MNN

如果您的策略模型当前是 `.onnx` 格式，可以先安装 MNN 转换工具，再执行模型转换：

```bash
python3 -m pip install -U MNN
python3 -m pip install -U onnx
MNNConvert -f ONNX --modelFile model.onnx --MNNModel model.mnn --bizCode MNN
```

其中 `model.onnx` 为输入模型，`model.mnn` 为输出模型名称，可按实际文件名替换。

若安装后系统中找不到 `MNNConvert`，也可以尝试通过 Python 模块方式调用：

```bash
python3 -m MNN.tools.mnnconvert -f ONNX --modelFile model.onnx --MNNModel model.mnn --bizCode MNN
```

### 2. 修改策略观测或动作

如果你的模型输入输出与示例不同，通常需要同步修改：

- `deploy_template/config/policy/.../params/deploy.yaml`
- `deploy_template/src/State_Mimic.cpp` 中注册的 observation
- `deploy_template/include/State_Mimic.h` 中的 `MotionLoader_`

建议先保持关节顺序、动作维度和 `joint_ids_map` 与示例一致，确认能稳定运行后再扩大改动范围。

### 3. 新增自己的控制状态

最稳妥的方式是复制 `State_Mimic.cpp` / `State_Mimic.h`，改成自己的状态类，例如 `State_MyController`：

1. 在头文件末尾添加 `REGISTER_FSM(State_MyController)`。
2. 在 `deploy_template/config/config.yaml` 的 `FSM._` 中启用新状态。
3. 在 `FSM` 下添加新状态的配置项。
4. 在 `main.cpp` 中包含你的状态头文件。
5. 在 `Passive` / `Exit` 等状态里确认触发条件会跳到你的状态。

如果你把状态名从 `Mimic` 改成别的名字，请全局搜索 `Mimic`。当前模板里有一些状态跳转和发布保护与这个名字相关，改名时需要一起更新。

## 安全与回退逻辑

模板里已经放了几层保护，但它不能替代实机测试规范：

- 进入 `Mimic` 后会尝试 `SwitchToUserCtrl()`，并在 0.1 秒后检查官方 FSM ID 是否为 `1000`。
- 如果没有成功进入用户控制模式，策略线程会退出并触发回退。
- `Mimic` 超过 `time_end` 或策略线程停止后，会跳到 `Exit`。
- 姿态偏差过大时，会跳到 `Passive`。
- `Exit` 会根据 IMU 姿态选择回到官方 `WALKRUN` 或 `PASSIVE`。

实机运行前建议：

- 先把 `time_start` / `time_end` 设置成 1 到 3 秒的短片段。
- 先在支架、吊装或足够安全的环境中测试。
- 确认 `motion_file`、`policy_file`、`deploy.yaml` 的关节顺序和维度完全匹配。
- 确认 UniStore 应用权限里包含 `motor`。
- 确认没有其他进程同时向低层电机指令 topic 发布冲突指令。

## 构建方式

### 推荐：使用 ARM64 Docker 构建

仓库根目录已经提供：

- `Dockerfile.arm64-builder`：只构建工具链镜像。
- `docker_build_arm64.sh`：运行容器时把当前仓库挂载到 `/workspace`，在容器里原地编译。

使用方式：

```bash
chmod +x docker_build_arm64.sh
./docker_build_arm64.sh
```

脚本会完成：

1. 构建 Ubuntu 20.04 ARM64 builder 镜像。
2. 安装并编译 `unitree_sdk2`。
3. 执行 `deploy_template/install.sh`。
4. 将产物同步到 `app/g1/app_out`。

如果你想修改产物同步目录：

```bash
APP_OUTPUT_DIR=/your/output/path ./docker_build_arm64.sh
```

如果你想指定 `unitree_sdk2` 分支：

```bash
docker build \
  --platform linux/arm64 \
  --build-arg UNITREE_SDK2_REF=main \
  -f Dockerfile.arm64-builder \
  -t g1-unistore-builder:ubuntu20.04-arm64 .
```

### 手动安装依赖

如果不用 Docker，需要在 Ubuntu 20.04 ARM64 环境中安装依赖：

```bash
sudo apt install -y libyaml-cpp-dev libboost-all-dev libeigen3-dev libspdlog-dev libfmt-dev
git clone git@github.com:unitreerobotics/unitree_sdk2.git
cd unitree_sdk2
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=OFF
sudo make install
```

然后进入 `deploy_template` 编译：

```bash
cd deploy_template
./install.sh
```

编译完成后，确保 `app/g1/app_out` 中包含：

```text
bin/g1_ctrl
config/
lib/
```

## 本地仿真测试

在仿真环境测试时，可在 `deploy_template/include/unitree/dds_wrapper/robots/g1/g1_pub.h` 中修改 `LowCmd` 的默认 topic 为`rt/lowcmd`，可以直接运行：

```bash
cd deploy_template
./run_g1_ctrl.sh
```

默认脚本使用 `./g1_ctrl -n lo`。如果你的 DDS 网络接口不是 `lo`，请根据实际网络环境修改 `deploy_template/run_g1_ctrl.sh`。

UniStore 包内运行时入口为：

```bash
app/g1/run_g1_ctrl.sh
```

## UniStore API 入口

`app/g1/config.py` 默认配置如下：

```python
EXECUTABLE_BIN = "/workspace/app/g1/run_g1_ctrl.sh"
REALTIME=True

OPTIONS = [
    {"id": 1, "name": "Start", "cmd": "s", "args": []},
]
```

如果你需要手动停止动作，可以按需增加 Stop 命令，例如向进程输入 `o`，并在 C++ 控制状态里处理对应按键。

## 应用元数据

填写 `metadata.yaml`。示例：

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

如果你的应用不使用相机，可以按 UniStore 要求移除 `camera` 权限；控制电机时需要保留 `motor`。

## 打包上传

构建完成并确认 `app/g1/app_out` 已生成后，在 `g1_mimic_demo` 目录下打包：

```bash
tar -czvf your_app_name.tar.gz ./app
```

之后将 `your_app_name.tar.gz` 上传到 UniStore。

## 常见问题

### Start 后没有进入用户控制

检查日志中的 `Current FSM ID`。模板会在进入用户控制后检查 FSM ID 是否为 `1000`；如果不是，通常说明当前官方运控状态不允许切到用户控制，目前仅支持从走跑模式进入，或者 `SwitchToUserCtrl()` 没有成功。

### 模型能启动但动作不对

优先检查 `motion_file`、`policy_file`、`deploy.yaml` 的关节顺序、动作维度、默认关节角和 history 配置是否与训练时一致。

### 想改成本地 topic 测试

UniStore 默认使用 `rt/user_lowcmd`，上传到官方UniStore平台必须使用此topic。本地测试可在 `deploy_template/include/unitree/dds_wrapper/robots/g1/g1_pub.h` 中修改 `LowCmd` 的默认 topic。

### 改了 FSM 名字后跑不起来

全局搜索旧状态名，例如 `Mimic`。需要同时检查 `config.yaml`、`Passive` / `Exit` 的跳转目标、状态类注册、`main.cpp` include，以及任何发布保护逻辑。
