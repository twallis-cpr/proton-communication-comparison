#!/usr/bin/env bash
set -euo pipefail

# The espressif/idf image's entrypoint sources export.sh for interactive shells,
# but postCreateCommand runs a fresh bash without it.
. "${IDF_PATH}/export.sh"

git config --global --add safe.directory "$(pwd)"
git submodule update --init --recursive

pip install --no-cache-dir catkin_pkg colcon-common-extensions lark

cat <<'EOF'

Devcontainer ready.

Next:
  cd micro_ros
  idf.py set-target esp32c6
  idf.py menuconfig     # Bench Configuration > WiFi + Transport Target
  idf.py build flash monitor

EOF
