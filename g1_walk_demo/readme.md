# 命令行程序示例模版
本项目提供了一个示例程序，演示如何将一个闭源的二进制程序快速封装为 `App` 并上传。

目标用户是任何开发者，只要能够**编写命令行程序**，即可轻松上手。

如果您来自 **ROS2、前后端、云原生、MCP 等其他领域**，也可参考对应文档，在自己熟悉的技术栈中零成本开发一个应用。

## 约束
`App` 使用 `docker` 作为基础设施，支持任意语言和框架，支持任意 `Linux` 发行版(`Debian、Alpine、Ubuntu22.04、Ubuntu20.04` 等)，因此您可以使用您熟悉的任何技术开发 `App`，发布只需要满足以下最小要求：
- 您的前端 `UI` 的请求路径改为 `/app_api/[app_name]`
- 您的后端提供服务的端口为 `80`

满足以上两点，您即可完全自由的使用任何技术开发您的应用。

## 快速开始

项目目录：
- `control` 为命令行程序源码目录，您可以替换为您自己的程序。
- `server` 为 `python` 服务例程，可以快速将一个命令行交互程序变为 `http` 服务。
- `ui` 目录为界面，帮助您快速在手机上提供界面。

当然，您也可以任意修改 `server` 和 `ui`，只需满足**约束**小节的内容。
### 配置程序

修改 `ui/config.json`，改为您自己的应用名称，示例如下：
```json
{
  "appUrl": "/app_api/g1_walk_demo"
}

```

修改 `server/config.py`，增加您的二进制程序支持的输入，示例如下：
```python
# 可执行文件路径
EXECUTABLE_BIN = "/workspace/app/cmd" 
# 您的命令行程序是否实时输入，如果通过命令行参数输入，置为 False 即可，实时响应为 True
REALTIME=False 
# 支持的按钮
OPTIONS = [
    {"id": 1, "name": "前进一步", "cmd": "move", "args": ["0.6", "0", "0"]},
    {"id": 2, "name": "后退一步", "cmd": "move", "args": ["-0.6", "0", "0"]},
    {"id": 3, "name": "向左平移", "cmd": "move", "args": ["0", "0.6", "0"]},
    {"id": 4, "name": "向右平移", "cmd": "move", "args": ["0", "-0.6", "0"]},
    {"id": 5, "name": "原地左转", "cmd": "move", "args": ["0", "0", "0.6"]},
    {"id": 6, "name": "原地右转", "cmd": "move", "args": ["0", "0", "-0.6"]},
]
```

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

执行以下命令构建一个应用包:
```bash
# 编译命令行程序
cd ./control
mkdir -p build && cd build && cmake .. && make -j4 && cd ../../

mkdir -p production/app

# 拷贝您的命令行程序到 app 目录
cp ./control/build/cmd ./production/app/

# 拷贝您的接口服务程序到 app 目录
cp -r ./server ./production/app/

# 拷贝 UI 到 production 目录
cp -r ./ui ./production/

# 打包
cd ./production
tar -czvf your_app_name.tar.gz app ui
```

之后将应用上传到 App Store 即可。
