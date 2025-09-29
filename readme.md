# 宇树应用模板

欢迎使用宇树应用模板仓库！本仓库旨在为开发者提供一个**开箱即用**的环境，帮助您快速上手宇树机器人的应用开发。无论您是想使用 Python、C++ 还是 ROS2 来编写应用，我们都为您准备了相应的示例代码和配置文件。

本仓库采用 Monorepo 结构，以下是本仓库的主要内容：

| 目录路径                        | 示例名称              | 语言 |主要功能描述                                         |
| ------------------------------- | --------------------- |-------- | ---------------------------------------------------- |
| `g1_walk_demo/`        | G1 行走控制 | `C++`主控，`python`提供服务 | 展示如何通过**命令行程序**控制 **G1**行走。 |

关于如何控制机器人的详细内容，您可以参考[宇树仓库](https://github.com/unitreerobotics) 中对应语言的 sdk。

## 快速开始

本指南将帮助您在数分钟内启动并运行宇树机器人的应用开发环境。

我们强烈推荐使用 **VS Code Dev Containers**，它能为您提供一个**预配置、干净且隔离**的环境。**然而，我们的应用发布流程与您的开发方式解耦。** 如果您不想使用 Docker 或 VS Code，您仍然可以**用您习惯的任何方式**开发和编译代码，只要最终能按照**约定**将应用打包成可识别的格式，就可以上传至机器人商店使用。

如果您选择不使用 Dev Containers，您可以：
- **本地安装依赖**： 确保您的本地环境（操作系统、编译器、SDK 依赖等）满足对应子项目 README 或 requirements.txt 中列出的所有要求。
- **自由开发**： 使用您偏好的 IDE（如 JetBrains 全家桶、Vim 等）或构建工具进行开发和调试。
- **最终打包**： 无论您采用何种开发方式，您的最终目标是按照宇树的应用发布**约定**编译和打包您的程序。只要最终产物符合上传要求，您就可以将其上传到机器人商店使用。

为方便开发者快速上手开发，以下步骤将以 **VS Code Dev Containers** 作为推荐的开发方式进行说明。

### 0. 开发容器环境要求

在开始之前，推荐您安装以下软件，当然您也可以使用其他近似产品：

1. **Git:** 用于克隆代码仓库。
2. **Docker:** 用于构建和运行容器环境。
3. **Visual Studio Code (VS Code):** 您的开发编辑器。
4. **VS Code 扩展:**
   - **Dev Containers (开发容器) 扩展**：搜索并安装 `ms-vscode-remote.remote-containers`。

### 1. 克隆代码仓库

打开您的终端，将应用模板仓库克隆到本地：

```bash
git clone https://github.com/unitreerobotics/unitree-app-templates.git
cd unitree-app-templates
```

### 2. 选择并启动开发环境（推荐方式）

每个示例项目都有一个独立的 Dev Container 配置，请根据您想运行的项目，进入对应的子目录，然后启动 VS Code。

#### 示例：启动 Python 控制示例

假设您的 Python 示例代码位于 `python-control-demo` 目录下：

1. **进入子项目目录：**

   ```bash
   cd python-control-demo
   ```

2. **启动 VS Code：**

   ```bash
   code .
   ```
   您也可以直接打开 VS Code，然后使用 `File -> Open Folder` 导航到该目录并打开它。

#### 3. 连接到容器

当 VS Code 启动后，它将检测到子目录中的 **`.devcontainer/devcontainer.json`** 配置文件，并在右下角弹出提示：

> 🚨 **"Folder contains a Dev Container configuration file. Reopen in Container?"**

请点击：**`Reopen in Container`**

VS Code 将自动完成以下步骤：

- 拉取或构建 Docker 镜像（其中已安装好所有 Python 依赖、宇树 SDK 等）。
- 将您的代码挂载到容器内。
- 重新加载 VS Code 窗口，您现在已连接到一个隔离、配置好的开发环境。

### 4. 运行您的应用代码

容器连接成功后，您无需在本地安装任何依赖，即可直接在 VS Code 终端中运行您的示例代码。

#### 示例：运行 Python Demo

在 VS Code 的集成终端中执行：

```bash
# 假设您的启动文件名为 main.py
python3 main.py
```

现在，您就可以开始在完全配置好的环境中调试和开发您的宇树应用了。

------

**提示：** 如果您想切换到另一个示例（例如 C++ 或 ROS2 示例），只需关闭当前的 VS Code 窗口，回到终端，`cd` 到新的示例目录，再次执行 `code .` 即可。