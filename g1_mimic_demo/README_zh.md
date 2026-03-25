# 示例说明

应用商店部署代码"deploy_template"以unitree_rl_lab为基础，示例状态机为Mimic，根据需要修改，若更改状态机名，则需要在 include/FSM/FSMState.h 的post_run函数中修改对应命名，以保证仅在此状态机时才发布lowcmd。

应用商店使用"rt/user_lowcmd"发布电机指令，本地测试可在 include/unitree/dds_wrapper/robots/g1/g1_pub.h 中修改话题。

# 安装引导

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

运行 ./docker_build_arm64.sh ，编译完成后，产物会存到app/g1文件夹下，并确保此目录下含有run_g1_ctrl.sh此类启动脚本

## 版本
填写您应用程序的元数据 `metadata.yaml`，示例如下:
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
  - 初始版本发布
```

## 打包
tar -czvf your_app_name.tar.gz ./app ./ui

之后将应用your_app_name.tar.gz上传到 UniStore 即可
