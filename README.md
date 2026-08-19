# esp32_proton_bench

ESP32-C6 firmware for comparing Clearpath [proton](https://github.com/clearpathrobotics/proton)
against micro-ROS. Transmits synthetic IMU and wheel-feedback data over Wi-Fi to
a fixed destination IP.

## Prerequisites

- ESP-IDF v6.0.1
- ESP32-C6 devkit

## Build

```sh
. $IDF_PATH/export.sh
idf.py set-target esp32c6
idf.py menuconfig    # set Wi-Fi SSID / password under "micro-ROS Settings > WiFi Configuration"
idf.py build
idf.py flash monitor
```

## micro-ROS

The [micro-ROS ESP-IDF component](https://github.com/micro-ROS/micro_ros_espidf_component)
is vendored as a git submodule under `micro_ros/components/micro_ros_espidf_component`
tracking the `rolling` branch.

Clone with submodules, or initialize after cloning:

```sh
git submodule update --init --recursive
```

Install the micro-ROS Python build dependencies **inside the IDF virtual environment**:

```sh
. $IDF_PATH/export.sh
pip3 install catkin_pkg colcon-common-extensions lark
```

In `idf.py menuconfig`, under **micro-ROS Settings**, select:

- Middleware: `micro-ROS over eProsima Micro XRCE-DDS`
- Network interface: `WLAN interface`
- Fill in **WiFi Configuration → SSID / Password**
- Set the agent host IP / port to your development machine

To wipe the micro-ROS build cache after upgrading the submodule or changing
middleware options:

```sh
idf.py clean-microros
```

Run a matching UDP4 agent on your dev machine:

```sh
docker run -it --rm --net=host microros/micro-ros-agent:rolling udp4 --port 8888 -v6
```
