#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <cmath>
#include <sensor_msgs/Imu.h>
#include <tf/transform_datatypes.h>

bool got_finish = false;
bool has_record_start = false;
double start_x = 0.0, start_y = 0.0;
double current_x = 0.0, current_y = 0.0;

bool turn_finish = false;
double yaw = 0.0;
double start_yaw = 0.0;
bool need_turn = false;
const double TARGET_ANGLE = 90.0;

int loop_cnt = 0;
const int MAX_LOOP = 4;

void odom_callback(const nav_msgs::OdometryConstPtr &odom_msg)
{
    current_x = odom_msg->pose.pose.position.x;
    current_y = odom_msg->pose.pose.position.y;

    if (!has_record_start)
    {
        start_x = current_x;
        start_y = current_y;
        has_record_start = true;
        ROS_INFO("start：(%.2f, %.2f)，目标走2米", start_x, start_y);
        ROS_INFO("startsign%.2f", (double)has_record_start);
        return;
    }

    double dist = sqrt(pow(current_x - start_x, 2) + pow(current_y - start_y, 2));
    ROS_INFO("start1：(%.2f, %.2f)，目标走2米", start_x, start_y);
    ROS_INFO("start2：(%.2f, %.2f)，已走：%.2f米", current_x, current_y, dist);
    ROS_INFO("startsign%.2f", (double)has_record_start);

    if (dist > 1.0 && !got_finish)
    {
        ROS_INFO("finish：(%.2f, %.2f)", start_x, start_y);
        got_finish = true;
        ROS_INFO("Finish！已走距离：%.2f米（目标2米）", dist);
    }
}

void imu_callback(const sensor_msgs::ImuConstPtr &imu_msg)
{
    double roll, pitch;
    tf::Quaternion q(
        imu_msg->orientation.x,
        imu_msg->orientation.y,
        imu_msg->orientation.z,
        imu_msg->orientation.w);
    tf::Matrix3x3(q).getRPY(roll, pitch, yaw);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "odom_listener");
    ros::NodeHandle nh;

    ros::Subscriber odom_sub = nh.subscribe("/odom", 10, odom_callback);
    ros::Subscriber imu_sub = nh.subscribe("/imu/data", 10, imu_callback);

    ros::Publisher cmd_pub_ = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    geometry_msgs::Twist cmd_vel;

    ros::Rate rate(20);

    while (ros::ok())
    {
        bool timeout = (ros::Time::now() - ros::Time::now().fromSec(ros::Time::now().toSec())).toSec() > 5.0;

        if (got_finish || timeout)
        {
            cmd_vel.linear.x = 0.0;
            cmd_vel.angular.z = 0.0;

            if (!need_turn && got_finish)
            {
                need_turn = true;
                start_yaw = yaw;
                ROS_INFO("=== start turn 90 ===");
            }

            if (need_turn && !turn_finish)
            {
                // ====================== 关键修复：IMU 180° 跳变问题 ======================
                double angle_diff_rad = yaw - start_yaw;
                // 自动处理 -π ~ π 跳变
                angle_diff_rad = atan2(sin(angle_diff_rad), cos(angle_diff_rad));
                double angle_diff = angle_diff_rad * 57.29578;
                // ======================================================================

                if (angle_diff < TARGET_ANGLE - 1.0)
                {
                    ROS_INFO("turn (start yaw)：(%.2f,%.2f) ", start_yaw, yaw);
                    cmd_vel.angular.z = 0.3;
                }
                else
                {
                    cmd_vel.angular.z = 0.0;
                    turn_finish = true;
                    ROS_INFO("turn 90");

                    loop_cnt++;
                    if (loop_cnt >= MAX_LOOP)
                    {
                        ROS_INFO("=== Square path done ===");
                        break;
                    }

                    got_finish = false;
                    has_record_start = false;
                    need_turn = false;
                    turn_finish = false;
                    ros::Duration(0.5).sleep();
                }
                ROS_INFO("angle: %.1f deg / target: %.1f deg", angle_diff, TARGET_ANGLE);
            }
        }
        else
        {
            cmd_vel.linear.x = 0.2;
            cmd_vel.angular.z = 0.0;
        }

        cmd_pub_.publish(cmd_vel);
        ros::spinOnce();
        rate.sleep();
    }

    cmd_vel.linear.x = 0.0;
    cmd_vel.angular.z = 0.0;
    cmd_pub_.publish(cmd_vel);

    return 0;
}

