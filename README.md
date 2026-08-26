# proton-communication-comparison

ESP32-C6 firmware for comparing Clearpath [proton](https://github.com/clearpathrobotics/proton) against several robotics and industrial protocols. Transmits synthetic IMU and wheel-feedback data over Wi-Fi to a fixed destination IP.

The goal is to measure the following metrics of each library:

- **size**: using `idf.py size-components`, determine how large each library is in terms of RAM and Flash. Note that this is for static RAM/Flash, dynamic allocations are not being considered for this test
- **network traffic**: each project is set up to transmit a synthetic IMU message to a fixed target. This mimics the application in most small robots where a small embedded controller is part of an internal network sending data to a host PC. The host PC is only set up to receive the message, and transmit back any data that is _required_ for the protocol to operate. Network data is tracked in Wireshark from the host PC, measured in bits/s over a 30 second window.

## Protocols Tested

- **proton**
- **micro-ROS**
- **zenoh-pico**
- **EtherNet/IP**: note: **network traffic only**

## Repository layout

- `components/bench_common/`: shared ESP-IDF component: Wi-Fi station bring-up, IMU task, and Kconfig entries (SSID/password, auth mode, destination IPv4/port). Both ESP-IDF projects consume it via `EXTRA_COMPONENT_DIRS`.
- `micro_ros/`: ESP-IDF project that transmits via the micro-ROS `micro_ros_espidf_component` (git submodule, `rolling` branch).
- `proton/`: ESP-IDF project that transmits via the proton protocol (git submodule, `2.0.0-beta6` tag)
- `zenoh/`: ESP-IDF project that transmits via the zenoh protocol using zenoh-pico (git submodule, `main` branch)
- `collected_data`: network captures and output of `idf.py size-components`

## Prerequisites

- ESP32 devkit. This project was developed using the ESP32-C6 devkit.

This project uses git submodules, after cloning, initialize via:

```sh
git submodule update --init --recursive
```

## Building

### Environment

This project is designed to run using ESP-IDF v6.0.1 in either a Docker container or devcontainer (included), using Espressif's ESP-IDF 6.0 image.

### Using the devcontainer (recommended)

1. Install the **Dev Containers** VS Code extension.
2. Plug in the ESP32-C6 devkit **before** opening the folder in the container (the container binds `/dev/ttyUSB0` at start; if the device isn't present, edit `.devcontainer/devcontainer.json` and remove the `--device` line, or change it to match your board's tty).
3. Run **Dev Containers: Reopen in Container**. First launch pulls the IDF image (~4 GB) and runs `.devcontainer/post-create.sh` to init submodules and install the Python deps into the IDF venv.
4. Open a terminal inside the container — `IDF_PATH` and the IDF venv are already sourced by the image entrypoint — and run:

    ```sh
    . ${IDF_PATH}/export.sh
    cd project-name
    idf.py set-target esp32c6
    idf.py menuconfig     # Bench Configuration > WiFi + Transport Target
    idf.py build flash monitor
    ```

### Using raw docker (fallback)

```sh
docker run --name esp-microros -it --device=/dev/ttyUSB0 espressif/idf:release-v6.0 bash # Replace /dev/ttyUSB0 with your ESP32's path
git clone --recurse-submodules https://github.com/twallis-cpr/proton-communication-comparison.git
cd proton-communication-comparison/micro_ros
pip install catkin_pkg colcon-common-extensions lark
idf.py set-target esp32c6 # Or whatever target you choose
idf.py menuconfig # to set your wifi credentials (see above)
idf.py build
idf.py flash
```

### Build Settings

**General Settings**
 - -Os optimisation (size)
 - Debug symbols stripped
 - linker set to `--gc-sections`
 - Full assertions (default)
 - LTO off (default)

Per-library settings are changed in order to get the project running on the ESP32. Additionally, per-library defines or settings are left as defaults where they can be, or as the default value in examples that closest match the project's intentions. IE, using the settings from micro-ROS' `int32_publisher` example, and zenoh-pico's `z_put` example.

## micro-ROS

Install the micro-ROS Python build dependencies **inside the IDF virtual environment**:

```sh
. $IDF_PATH/export.sh
pip3 install catkin_pkg colcon-common-extensions lark
```

Configure, build and flash:

```sh
. $IDF_PATH/export.sh
cd micro_ros
idf.py set-target esp32c6
idf.py menuconfig    # set Wi-Fi SSID / password under "Bench Configuration > WiFi Configuration"
idf.py build
idf.py flash monitor
```

In `idf.py menuconfig`:

- Under **Bench Configuration**: enter Wi-Fi SSID/password and destination IPv4/port. Both ESP-IDF projects read these.
- Under **micro-ROS Settings**: select `micro-ROS over eProsima Micro XRCE-DDS`. Leave the network-interface choice unselected

### Wi-Fi ownership

Wi-Fi bring-up is done entirely by `components/bench_common/wifi_sta.c` (called from `app_main`), **not** by the micro-ROS component's `uros_network_interface_initialize()`. All Wi-Fi Kconfig entries live under  Bench Configuration > WiFi Configuration` (symbols `CONFIG_BENCH_WIFI_*`), so the micro-ROS component's own `WLAN interface` selection is unnecessary and should be left off to avoid duplicate Wi-Fi init at run time.

Do **not** call `uros_network_interface_initialize()` from application code. `wifi_sta.c` already registers the default event loop, STA netif, and event handlers, and a second bring-up would fail. When micro-ROS is wired in, pass the agent endpoint explicitly via `rmw_uros_options_set_udp_address()` using `CONFIG_BENCH_TARGET_IPV4` and `CONFIG_BENCH_TARGET_PORT`.

To wipe the micro-ROS build cache after upgrading the submodule or changing middleware options:

```sh
idf.py clean-microros
```

Run a matching UDP4 agent on your dev machine (port must match `CONFIG_BENCH_TARGET_PORT`):

```sh
docker run -it --rm --net=host microros/micro-ros-agent:rolling udp4 --port 8888 -v6
```

### Reading data with uros agent

```sh
docker run -it --rm --net=host microros/micro-ros-agent:rolling udp4 --port 8888 -v6
```

## Proton

Proton should build and install without the aid of a devcontainer or Docker, but the micro-ROS image works just fine.

### Building

```sh
. $IDF_PATH/export.sh
cd proton
idf.py set-target esp32c6
idf.py menuconfig    # set Wi-Fi SSID / password under "Bench Configuration > WiFi Configuration"
idf.py build
idf.py flash monitor
```

### Reading data with proton_ros2_node

`proton_ros2_node` v2.0 is still unreleased, so it must be built in a ROS 2 workspace with the clearpath apt mirror for a couple dependency packages.

To install the clearpath apt mirror, view the steps in our documentation, under the [Manual Source Install](https://docs.clearpathrobotics.com/docs/ros2humble/ros/installation/robot/#clearpath-package-server) section.

Specifically, `Clearpath Package Server`.

Install the following prerequisite package:

```sh
sudo apt install ros-jazzy-serial-hardware
```

Then create a workspace

```sh
mkdir -p /path/to/proton_ws/src
cd proton_ws/src
git clone https://github.com/clearpathrobotics/proton_ros2.git
git clone --branch feature/proton-2.0.0-beta https://github.com/clearpathrobotics/proton_vendor.git
cd /path/to/proton_ws
colcon build
. install/local_setup.bash
```

In order to broadcast the IMU data as a ROS 2 `sensor_msgs/msg/Imu` message, you will need to create a proton adaptor package with the correct signal mapping. With the workspace built and installed

```sh
cd /path/to/proton_ws/src
ros2 run proton_ros2_adaptor_generator proton_ros2_adaptor_generator generate -p imu_bridge_adaptor -c /path/to/this/repository/proton/components/proton_core/config/proton_bench.yaml --output ./imu_bridge --project-name 'imu_bridge'
cd ../
colcon build --packages-select imu_bridge_adaptor
. install/local_setup.bash
```

Then run `proton_ros2_node`

```sh
ros2 launch proton_ros2_node proton_ros2_node.launch.py proton_config_file:=/path/to/this/repository/proton/components/proton_core/config/proton_bench.yaml binding_config_file:=/path/to/this/repository/proton/components/proton_core/config/proton_bench.yaml target:=pc
```

## zenoh-pico

TBD

## Component size comparison

To read how large each component is, run

```sh
idf.py size-components
```

and search for the component you want to see (libmicroros.a, libproton.a, etc)
