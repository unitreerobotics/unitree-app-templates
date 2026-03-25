#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="${DOCKER_IMAGE_NAME:-g1-unistore-builder:ubuntu20.04-arm64}"
APP_DIR="${APP_OUTPUT_DIR:-${ROOT_DIR}/app/g1/app_out}"

docker build \
  --platform linux/arm64 \
  -f "${ROOT_DIR}/Dockerfile.arm64-builder" \
  -t "${IMAGE_NAME}" \
  "${ROOT_DIR}"

docker run --rm \
  --platform linux/arm64 \
  -e CONFIG_ENC_KEY="${CONFIG_ENC_KEY:-unitree-g1-config-key}" \
  -v "${ROOT_DIR}:/workspace" \
  -w /workspace/deploy_template \
  "${IMAGE_NAME}" \
  ./install.sh

mkdir -p "${APP_DIR}"
rsync -a --delete "${ROOT_DIR}/deploy_template/out/" "${APP_DIR}/"

echo "Build finished."
echo "Artifacts synced to: ${APP_DIR}"
