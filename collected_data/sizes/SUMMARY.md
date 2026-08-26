# Firmware size comparison — micro-ROS vs. proton

## Overview

To compare size and memory resource usage of micro-ROS vs. Clearpath proton.

The bench firmware connects to a Wi-Fi network at a static IP, and transmits simulated IMU data over its respective communication protocol. The IMU data represents the kind of data found on a commodity IMU: angular velocity and linear acceleration over 3 axes each. Also simulated is are the covariances for these two measurements. For simulation purposes, the data on each axis is different offsets of a sine-wave, with the covariance matching the X axis of the respective sensor.

Simulated IMU data is generated in a dedicated task (`imu_gen_task`) and transmitted over a FreeRTOS queue at 100Hz using a neutral datatype struct (`ImuData_t`). Each communication protocol runs in its own task at 100Hz, pulling data from the queue, copying the data into its respective message type, and transmits it to a fixed IP address on the same network.

## Test Setup

### Hardware
  - Board: ESP32-C6-DevKitM-1
  - Host PC: Ubuntu 24.04, ROS 2 Jazzy

### Networking
  - ESP32 (wifi): a dumpy router from home, 300Mbps Wireless-N
  - Host PC (LAN): Ethernet connected to same router

### Build Settings
 - -Os optimisation (size)
 - Debug symbols stripped
 - linker set to `--gc-sections`
 - Full assertions (default)
 - LTO off (default)

## Firmware build environment

Both projects share hardware, toolchain, and every ESP-IDF knob that meaningfully affects code size. The only application-level differences are the transmit component (proton_core vs. micro_ros_espidf_component) and its associated task stack.

### Toolchain / target

| | |
|---|---|
| Target chip | ESP32-C6 (RISC-V, single core) |
| Devkit | ESP32-C6-DevKitM-1 (4 MB flash physical) |
| ESP-IDF | v6.0.2 (`71f4b17`) |
| Toolchain | `riscv32-esp-elf-gcc` (bundled with IDF v6.0) |

### Flash / partition

| | |
|---|---|
| Configured flash size | 2 MB (`CONFIG_ESPTOOLPY_FLASHSIZE="2MB"`) — intentionally smaller than the 4 MB devkit so builds are portable to any ≥2 MB C-series chip |
| Partition table | Default `partitions_singleapp.csv` (`CONFIG_PARTITION_TABLE_SINGLE_APP=y`) |
| App partition | 1 MB `factory` slot |
| Bootloader | Default 2nd-stage, log level INFO |

### RTOS / logging

| | |
|---|---|
| FreeRTOS tick | 1 kHz (`CONFIG_FREERTOS_HZ=1000`) |
| `app_main` stack | 6144 B (`CONFIG_ESP_MAIN_TASK_STACK_SIZE`) |
| Log level | INFO (`CONFIG_LOG_DEFAULT_LEVEL_INFO=y`) |
| Panic behavior | Print + reboot (default) |
| Task WDT | Enabled, 5 s (default) |

### Wi-Fi

| | |
|---|---|
| Auth mode | WPA2-PSK (`CONFIG_BENCH_WIFI_AUTH_WPA2_PSK=y`) |
| WPA3 OWE, SAE-PK, SAE-H2E | Enabled (IDF v6 defaults) |
| BT / NimBLE / OpenThread | Off |
| Bring-up | `components/bench_common/wifi_sta.c` — shared between both projects; supports optional static IP (`/24`, no gateway) |
| Static IP path | proton passes its generated MCU endpoint; micro_ros passes `NULL` (DHCP) |

### Transmit-side components

| | proton | micro-ROS | Notes | 
|---|---|---|---|
| Component source | git submodule at `proton/components/proton_core/proton` | git submodule at `micro_ros/components/micro_ros_espidf_component` | |
| Version | tag `2.0.0-beta6` | `rolling` branch, currently `6.0.0-25-geecadcf` ||
| Middleware | proton static registry (generated from YAML) | eProsima Micro XRCE-DDS ||
| Transport | UDP4 via lwIP `sendto`, non-blocking | UDP4 via micro-ROS `rmw_uros_options_set_udp_address` ||
| Bench task stack | 3072 B (`CONFIG_PROTON_TASK_STACK`) | 16000 B (`CONFIG_MICRO_ROS_APP_STACK`) | 16kB for micro-ROS was the default according to the upstream submodule |
| Bench task priority | 5 | 5 | | 
| Payload cadence | 10 ms period (100 Hz) | rclc timer-driven (matches proton period) | |

## Summary

### Flash usage

| Metric | micro-ROS | proton | Delta |
|---|---|---|---|
| App-partition image (.bin) |944,992 B | 836,416 B | −108,576 B (−11.5%) |
| Free in 1 MB factory partition |10% | 20% | +10 pp |
| Application-stack code (own libs) | 107,818 B | 6,156 B | −101,662 B (~17.5× smaller) |


### Application-stack contribution

| Archive | Size | Notes |
|---|---|---|
| libmicroros.a | 107,818 B | rcl + rclc + rmw_microxrcedds + tinycbor + generated msg support |
| libproton_core.a | 4,724 B | proton runtime (encode/decode, node manager, transports) |
libproton_registry.a | 1,432 B | generated static registry (signals + bundles + node) |

### RAM

| Component | .bss + .data | Notes |
|---|---|---|
| libmicroros.a | 30,523 B (DIRAM) | Session tables, executor queues, publisher/subscriber slots |
| libproton_core.a | 0 B | All state lives in the registry / node structs owned by the app |
| libproton_registry.a | 1,372 B (DIRAM) | Signal + bundle state buffers |

micro-ROS costs ~30 KB of static RAM on top of stack usage; proton's static registry is ~1.4 KB. Actual heap use at runtime isn't captured in size-components, but micro-ROS's rclc_support_init + executor add several more KB of heap under middleware defaults. Proton has no heap allocation.

## Caveats

### This is a minimal implementation of micro-ROS

Numbers reflect a single-message bench (one IMU-like bundle, one publish/tx rate). micro-ROS's floor doesn't grow linearly with more topics. The middleware overhead is largely fixed, so at 10+ topics the ratio shrinks. proton grows very sub-linearly too (one row per signal in the generated registry).

micro-ROS has many more features than proton, it's a full rcl implementation for embedded systems. proton doesn't aim to accomplish this task, so only a subsection of micro-ROS's capabilities were selected to have as even of a comparison as possible for a limited test. As micro-ROS's extra functionalities are added, there will be more flash used (it's currently being stripped by `--gc-sections`) but I did not test _how_ much is being added.

### I'm explicitly not testing latency with this bench test

Latency in an embedded system is defined more by overall architecture than it is by which communication protocol you use. Just because one protocol has DDS under the hood, and the other has nothing, doesn't mean that one is more "real-time" than the other. Latency requirements are a conversation at the system level, not necessarily "make this go as fast as possible."

Additionally, I'm testing over a very old, low-performance router that was just lying around, and wifi has enough latency jitter going on that doing benchmarks is going to be counter-productive. For a more informative test, ethernet should be used to reduce the amount of variables, but I figured that an ESP32 over wifi is a more user-accessible board than a devkit with ethernet.
