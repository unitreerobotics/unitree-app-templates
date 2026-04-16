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
Run ./docker_build_arm64.sh. After the build completes, the artifacts will be placed in the app/g1 directory; ensure this directory contains a startup script such as run_g1_ctrl.sh.

## ONNX to MNN

If your policy model is currently in `.onnx` format, install the MNN converter first and then run the conversion:

```bash
python3 -m pip install -U MNN
python3 -m pip install -U onnx
MNNConvert -f ONNX --modelFile model.onnx --MNNModel model.mnn --bizCode MNN
```

Here, `model.onnx` is the input model and `model.mnn` is the output model name. Replace them with your actual filenames as needed.

If `MNNConvert` is not found after installation, you can also try invoking it through the Python module entry point:

```bash
python3 -m MNN.tools.mnnconvert -f ONNX --modelFile model.onnx --MNNModel model.mnn --bizCode MNN
```

## Application Metadata
Fill in the application metadata in metadata.yaml. Example:
```yaml
version: v0.1.0
base_image: image/sdk2_cpp_py:latest
permissions:
  - camera
build_env:
  - apt update && apt install -y python3 pip && pip install fastapi uvicorn -i https://pypi.tuna.tsinghua.edu.cn/simple

startup_commands:
  - python3 /workspace/app/g1/cmd_server.py
change_log: |
  - Initial release
```

## Pack
```yaml
tar -czvf your_app_name.tar.gz ./app
```

Finally, upload your_app_name.tar.gz to UniStore.
