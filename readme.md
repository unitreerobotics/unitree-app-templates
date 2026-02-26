# Unitree App Templates

Welcome to the Unitree App Templates repository!

This repository provides an out-of-the-box development environment to help developers quickly get started with building applications for Unitree robots. Whether you prefer Python, C++, Rust, or ROS2, we provide example projects and configurations to accelerate your development process.

---

## Repository Structure

This repository follows a monorepo structure. The main components are described below.

---

## Example Projects

| Directory Path     | Example Name        | Language                                   | Description |
|-------------------|--------------------|--------------------------------------------|------------|
| `g1_walk_demo/`     | G1 Walking Control | `C++` main controller, `Python` service        | Demonstrates how to control G1 walking via a command-line application. |
| `go2_action_demo/`  | Go2 Motion Control | `Python` main controller, `Python` service     | Demonstrates how to control Go2 movements using python_sdk2. |
| `go2_camera_demo/`  | Go2 Camera Viewer  | `Python` main controller, `Python` service     | Demonstrates how to retrieve and display Go2 camera images using python_sdk2. |

For detailed robot control documentation, please refer to the official Unitree SDK repositories:
https://github.com/unitreerobotics

---

## Utility Tools

| Directory Path | Tool Purpose        | Description |
|---------------|--------------------|------------|
| `mobile_viewer/` | UI Debugging       | Provides a lightweight HTTP server for debugging mobile UI rendering. |
| `unitree_cli/`   | Version Upload Tool | Provides a CLI tool for packaging and uploading application versions from the command line. |

---

## Application Packaging Requirements

Applications are deployed using Docker as the underlying infrastructure.

- Any programming language or framework is supported.
- Any Linux distribution is supported (e.g., Debian, Alpine, Ubuntu 22.04, Ubuntu 20.04, etc.).

You are free to use any technology stack you are comfortable with. To publish your app, you only need to satisfy the following minimum requirements:

| File/Folder     | Required | Description |
|----------------|----------|------------|
| `app/`           | Yes      | Application folder containing binaries or scripts required to run the program. The entry point must be defined in metadata.yaml. The app must provide an HTTP service on port 80. |
| `ui/`            | Optional | Frontend UI files. Requests are routed under /app_api/[app_name] and automatically forwarded to your app’s port 80. |
| `mcp/`           | Optional | Reserved for future support (e.g., voice interaction and AI integration). |
| `metadata.yaml`  | Yes      | Application metadata file containing basic app information and configuration. |

Note: Future versions may support bundling metadata.yaml together with the app. It is currently separated for easier debugging and modification.

---

# Quick Start

We strongly recommend using VS Code Dev Containers, which provide a pre-configured, clean, and isolated development environment.

However, the publishing workflow is decoupled from your development workflow. You are free to develop using any method you prefer, as long as the final packaged application follows the required conventions.

If you choose not to use Dev Containers:

- Install dependencies locally (ensure your OS, compiler, SDK, etc., satisfy each subproject’s README or requirements.txt).
- Develop freely using your preferred IDE (JetBrains, Vim, etc.).
- Package according to the publishing specification before uploading to the robot app store.

The steps below demonstrate the recommended VS Code Dev Container workflow.

---

## 0. Development Environment Requirements

Before starting, install the following tools (alternatives are acceptable):

1. Git – Clone the repository.
2. Docker – Build and run container environments.
3. Visual Studio Code (VS Code) – Code editor.
4. VS Code Extension:
   - Dev Containers (ms-vscode-remote.remote-containers)

---

## 1. Target Architecture & Cross-Platform Development

Official development containers support:

- amd64
- arm64

You can develop directly on machines with matching architectures.

Important Requirement:
Binaries uploaded to the app store must be compiled for the target robot architecture, typically arm64.

### Cross-Platform Development Options

If you do not have access to an arm64 machine:

Option 1 – Cross-Architecture Container Compilation (Recommended)

Docker images include built-in QEMU support.

This allows you to:
- Run an arm64 container on an amd64 machine
- Compile native arm64 binaries directly inside the container
- Achieve a seamless local dev → cross-compile → upload workflow

This approach maximizes the benefit of Unitree’s preconfigured environment and avoids complex toolchain setup.

Option 2 – Traditional Cross-Compilation

Install a cross-compilation toolchain locally (e.g., aarch64-linux-gnu-*) and compile manually.

---

## Current Robot Architectures

| Robot Model | Architecture |
|------------|-------------|
| G1         | arm64 |
| Go2        | arm64 |

---

## 2. Clone the Repository

```bash
git clone https://github.com/unitreerobotics/unitree-app-templates.git
cd unitree-app-templates
```

---

## 3. Select and Launch a Development Environment (Recommended)

Each example project includes its own Dev Container configuration.

Navigate into the project directory and launch VS Code:

```bash
cd g1_walk_demo
code .
```

Alternatively, open VS Code manually and select:
`File → Open Folder`

---

## 4. Reopen in Container

When VS Code detects **`.devcontainer/devcontainer.json`**, it will prompt:

> 🚨 **"Folder contains a Dev Container configuration file. Reopen in Container?"**

Click: **`Reopen in Container`**

VS Code will automatically:
- Pull/build the Docker image
- Install dependencies (Python, SDK, etc.)
- Mount your source code into the container
- Reload the workspace inside the isolated environment

You are now connected to a fully configured development environment.

---

## 5. Run Your Application

Once connected, you can directly run example code inside the VS Code terminal without installing dependencies locally.

Example (Python demo):

```bash
python3 ./server/cmd_server.py
```

You can now begin debugging and developing your Unitree application.

---

## 6. Debug Your Application

To debug the mobile UI:

- Navigate to the mobile_viewer directory
- Run the built-in HTTP server

See:
[mobile_viewer/README.md](./mobile_viewer/readme.md)

TODO: Real-robot debugging instructions will be provided in future updates.

---

## 7. Upload Your Application

CLI tool is under development. Please upload via the web interface for now.

After development and testing, use the provided CLI tool to package and upload your application:

See:
[unitree_cli/README.md](./unitree_cli/readme.md)

---

## Switching Between Examples

To switch to another example (e.g., C++ or ROS2):

1. Close the current VS Code window
2. Navigate to the new example directory
3. Run: `code .`

