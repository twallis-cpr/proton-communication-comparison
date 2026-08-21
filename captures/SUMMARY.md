# Bandwidth Comparison

This test recorded one-sided communication of the IMU message for each protocol. The idea was to determine the amount of data required to essentially transmit the same data. Not identical payloads, but what that data represents: a basic IMU message.

The test recorded 30s of data using Wireshark, started and stopped while data is transmitting in steady-state

## Bits/s

### micro-ROS

![micro-ROS bits per second](images/micro_ros_imu.png)

### proton
![proton bits per second](images/proton_imu.png)

Based on exported data, micro-ROS uses an average of 363166 bits/s, vs proton's 143305 bits/s. proton uses approx 39.4% of the same raw bandwidth to send the same data.

This is largely chalked up to the fact that proton only sends as much data as it needs to. micro-ROS uses the ROS standard serialization (effectively none) and ends up transmitting the entire sensor_msgs/msg/Imu message, which is 340 bytes long. That includes three 9-element covariances, and the unused 4-element orientation message.

proton inherits protobuf's varint encoding, meaning that data is compressed slightly, and only sends the gyro, accel, and a single value for their covariances, which can be used as a coefficient for the covariance matrix in the ROS bridging layer.
