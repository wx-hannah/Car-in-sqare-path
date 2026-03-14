#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <cmath>

int main(int argc, char **argv)
{
  ros::init(argc, argv, "square_loop_node");
  ros::NodeHandle nh;
  ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
  ros::Rate loop_rate(10);
  ros::Duration(1.0).sleep();

  geometry_msgs::Twist vel_msg;

  // 走 4 条边，形成正方形
  for (int i = 0; i < 4; i++) {

    // 1. 前进
    vel_msg.linear.x = 0.2;
    vel_msg.angular.z = 0.0;
    for (int j = 0; j < 40; j++) {
      pub.publish(vel_msg);
      ros::spinOnce();
      loop_rate.sleep();
    }

    // 停一下
    vel_msg.linear.x = 0.0;
    pub.publish(vel_msg);
    ros::Duration(0.5).sleep();

    // 2. 左转90度
    vel_msg.angular.z = 0.5;
    for (int j = 0; j < 38.5; j++) {
      pub.publish(vel_msg);
      ros::spinOnce();
      loop_rate.sleep();
    }

    // 停转
    vel_msg.angular.z = 0.0;
    pub.publish(vel_msg);
    ros::Duration(0.5).sleep();
  }

  // 最终停止
  vel_msg.linear.x = 0.0;
  vel_msg.angular.z = 0.0;
  pub.publish(vel_msg);

  return 0;
}