# 小车走1*1方格
第一节课第二个实验 2026.3.7

# 新终端启动底盘
roslaunch upros_bringup bringup_w2a.launch

# 编译此节点
cd wx_ros_class_ws

catkin_make

source devel/setup.bash

roscd my_class_pkg

# 开环控制走方格
打开新终端
rosrun my_class_pkg move_forward
# 闭环控制走方格
#打开新终端
rosrun my_class_pkg odom_move
