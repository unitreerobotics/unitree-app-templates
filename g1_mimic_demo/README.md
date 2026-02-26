# Example Description

The unistore deployment code 'deploy_template' is based on unitree_rl_lab.
The example finite state machine (FSM) used is Mimic. You can modify it as needed.
If you change the FSM name, make sure to update the corresponding name in the post_run function in
include/FSM/FSMState.h, so that lowcmd are published only when this FSM is active.

In the unistore version, motor commands are published via the topic "rt/user_lowcmd".
For local testing, the topic can be modified in
include/unitree/dds_wrapper/robots/g1/g1_pub.h.

# Installation Guide
```yaml
# Install dependencies
sudo apt install -y libyaml-cpp-dev libboost-all-dev libeigen3-dev libspdlog-dev libfmt-dev
# Install unitree_sdk2
git clone git@github.com:unitreerobotics/unitree_sdk2.git
cd unitree_sdk2
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=OFF # Install on the /usr/local directory
sudo make install
```
Run:  ./install.sh in a Ubuntu 20.04 ARM64 Docker environment.
After compilation, copy the out directory into the app/g1 folder, and make sure this directory contains a startup script such as run_g1_ctrl.sh.

## Application Metadata
Fill in the application metadata in metadata.yaml. Example:
```yaml
version: v0.1.0
base_image: image/cpp_sdk2:latest
permissions:
  - camera
build_env:
  - apt update && apt install -y python3 pip && pip install fastapi uvicorn -i https://pypi.tuna.tsinghua.edu.cn/simple

startup_commands:
  - python3 /workspace/app/server/cmd_server.py
change_log: |
  - Initial release
```

## Pack
```yaml
tar -czvf your_app_name.tar.gz ./app ./ui
```

Finally, upload your_app_name.tar.gz to UniStore.