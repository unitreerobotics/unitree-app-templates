# Unitree App Templates

Welcome to the Unitree App Templates repository!

This repository provides an out-of-the-box development environment to help developers quickly get started with building applications for Unitree robots.

## Repository Structure

This repository follows a monorepo structure. The main components are described below.

### Example Projects

| Directory Path | Example Name | Language | Description |
|----------------|--------------|----------|-------------|
| `g1_mimic_demo/` | G1 Mimic Learning Demo | `C++` main controller, `Python` service | Demonstrates how to use mimic learning policies to control **G1** to perform actions (e.g., Gangnam Style, Kaichangquan, etc.). |

For detailed robot control documentation, please refer to the official Unitree SDK repositories:
https://github.com/unitreerobotics

### Application Packaging Requirements

Applications are deployed using Docker as the underlying infrastructure. Any programming language or framework is supported. To publish your app, you only need to satisfy the following minimum requirements:

| File/Folder | Required | Description |
|-------------|----------|-------------|
| `app/` | Yes | Application folder containing binaries or scripts required to run the program. The entry point must be defined in metadata.yaml. The app must provide an HTTP service on port 80. |
| `metadata.yaml` | Yes | Application metadata file containing basic app information and configuration. |

## Quick Start

### Running G1 Mimic Learning Demo

Navigate to the directory and start the service:

```bash
cd g1_mimic_demo
python3 ./app/g1/cmd_server.py
```

For detailed instructions, please refer to [`g1_mimic_demo/README.md`](./g1_mimic_demo/README.md).

### Packaging

```bash
tar -czvf your_app_name.tar.gz ./app ./ui
```

Finally, upload `your_app_name.tar.gz` to UniStore.
