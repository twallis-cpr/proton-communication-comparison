#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <sensor_msgs/msg/imu.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
#include <rmw_microros/rmw_microros.h>
#endif

#include "data_types.h"
#include "imu_gen_task.h"
#include "wifi_sta.h"

static char *TAG = "esp32_uros_bench";

#define BENCH_STR_(x) #x
#define BENCH_STR(x)  BENCH_STR_(x)

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){ESP_LOGE(TAG, "Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){ESP_LOGE(TAG, "Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

rcl_publisher_t publisher;
sensor_msgs__msg__Imu msg;

QueueHandle_t imu_queue = NULL;

void timer_callback(rcl_timer_t * timer, int64_t last_call_time, uintptr_t arg)
{
	RCLC_UNUSED(last_call_time);
	RCLC_UNUSED(arg);

	if (timer != NULL && imu_queue != NULL) {
		ImuData_t imu_data;
		if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
			msg.angular_velocity.x = imu_data.angular_vel_x;
			msg.angular_velocity.y = imu_data.angular_vel_y;
			msg.angular_velocity.z = imu_data.angular_vel_z;

			msg.linear_acceleration.x = imu_data.linear_accel_x;
			msg.linear_acceleration.y = imu_data.linear_accel_y;
			msg.linear_acceleration.z = imu_data.linear_accel_z;

			int64_t uptime_us = esp_timer_get_time();
			msg.header.stamp.sec = uptime_us / 1000000LL;
			msg.header.stamp.nanosec = (uptime_us % 1000000LL) * 1000LL;
		}
		ESP_LOGI(TAG, "Publishing...");
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
	}
}

void micro_ros_task(void * arg)
{
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	RCCHECK(rcl_init_options_init(&init_options, allocator));

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
	rmw_init_options_t* rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

	// Static Agent IP and port can be used instead of autodisvery.
	RCCHECK(rmw_uros_options_set_udp_address(CONFIG_BENCH_TARGET_IPV4, BENCH_STR(CONFIG_BENCH_TARGET_PORT), rmw_options));
	//RCCHECK(rmw_uros_discover_agent(rmw_options));
#endif

	// create init_options
	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "esp32_imu_publisher", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
		"esp32_imu_publisher"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 10;
	RCCHECK(rclc_timer_init_default2(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback,
		true));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	const size_t tag_len = strlen(TAG);
	msg.header.frame_id.data = TAG;
	msg.header.frame_id.capacity = tag_len + 1;
	msg.header.frame_id.size = tag_len;

	while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
		usleep(10000);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node));
	RCCHECK(rcl_node_fini(&node));

  	vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "esp32_uros_bench booting");

    esp_err_t err = wifi_sta_start_and_wait(portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "wifi connect failed (%s), continuing without network",
                 esp_err_to_name(err));
    }

	imu_queue = xQueueCreate(4, sizeof(ImuData_t));

	if (imu_queue != NULL) {
		//pin micro-ros task in APP_CPU to make PRO_CPU to deal with wifi:
		xTaskCreate(micro_ros_task,
				"uros_task",
				CONFIG_MICRO_ROS_APP_STACK,
				(void*)imu_queue,
				CONFIG_MICRO_ROS_APP_TASK_PRIO,
				NULL);

		imu_gen_task_start((void*)imu_queue);
	} else {
		ESP_LOGE(TAG, "IMU queue is null, time to explode!");
		ESP_ERROR_CHECK(-1);
	}
}
