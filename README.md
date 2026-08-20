# esp32_proton_bench

ESP32-C6 firmware for comparing Clearpath [proton](https://github.com/clearpathrobotics/proton)
against micro-ROS. Transmits synthetic IMU and wheel-feedback data over Wi-Fi to
a fixed destination IP.

## Repository layout

- `components/bench_common/` — shared ESP-IDF component: Wi-Fi station bring-up,
  RNG/payload task, and Kconfig entries (SSID/password, auth mode, destination
  IPv4/port). Both ESP-IDF projects consume it via `EXTRA_COMPONENT_DIRS`.
- `micro_ros/` — ESP-IDF project that transmits via the micro-ROS
  `micro_ros_espidf_component` (git submodule, `rolling` branch).
- `proton/` — reserved for the future ESP-IDF project that transmits via
  Clearpath's Proton (not yet scaffolded).

## Prerequisites

- ESP-IDF v6.0.1
- ESP32-C6 devkit

## Build (micro-ROS project)

```sh
. $IDF_PATH/export.sh
cd micro_ros
idf.py set-target esp32c6
idf.py menuconfig    # set Wi-Fi SSID / password under "Bench Configuration > WiFi Configuration"
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

In `idf.py menuconfig`:

- Under **Bench Configuration**: enter Wi-Fi SSID/password and destination
  IPv4/port. Both ESP-IDF projects read these.
- Under **micro-ROS Settings**: select `micro-ROS over eProsima Micro XRCE-DDS`.
  Leave the network-interface choice unselected (or on UART) — see below.

### Wi-Fi ownership

Wi-Fi bring-up is done entirely by `components/bench_common/wifi_sta.c` (called
from `app_main`), **not** by the micro-ROS component's
`uros_network_interface_initialize()`. All Wi-Fi Kconfig entries live under
`Bench Configuration > WiFi Configuration` (symbols `CONFIG_BENCH_WIFI_*`), so
the micro-ROS component's own `WLAN interface` selection is unnecessary and
should be left off to avoid duplicate Wi-Fi init at run time.

Do **not** call `uros_network_interface_initialize()` from application code —
`wifi_sta.c` already registers the default event loop, STA netif, and event
handlers, and a second bring-up would fail. When micro-ROS is wired in, pass
the agent endpoint explicitly via `rmw_uros_options_set_udp_address()` using
`CONFIG_BENCH_TARGET_IPV4` and `CONFIG_BENCH_TARGET_PORT`.

To wipe the micro-ROS build cache after upgrading the submodule or changing
middleware options:

```sh
idf.py clean-microros
```

Run a matching UDP4 agent on your dev machine (port must match
`CONFIG_BENCH_TARGET_PORT`):

```sh
docker run -it --rm --net=host microros/micro-ros-agent:rolling udp4 --port 8888 -v6
```

## Running micro-ros build

The micro-ROS component only builds reliably inside the official ESP-IDF
Docker image. A VS Code devcontainer is provided in `.devcontainer/` that wraps
`espressif/idf:release-v6.0`, mounts `/dev/ttyUSB0` for flashing, and installs
the micro-ROS Python build dependencies (`catkin_pkg`,
`colcon-common-extensions`, `lark`) on first launch.

### Using the devcontainer (recommended)

1. Install the **Dev Containers** VS Code extension.
2. Plug in the ESP32-C6 devkit **before** opening the folder in the container
   (the container binds `/dev/ttyUSB0` at start; if the device isn't present,
   edit `.devcontainer/devcontainer.json` and remove the `--device` line, or
   change it to match your board's tty).
3. Run **Dev Containers: Reopen in Container**. First launch pulls the IDF
   image (~4 GB) and runs `.devcontainer/post-create.sh` to init submodules
   and install the Python deps into the IDF venv.
4. Open a terminal inside the container — `IDF_PATH` and the IDF venv are
   already sourced by the image entrypoint — and run:

    ```sh
    cd micro_ros
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
