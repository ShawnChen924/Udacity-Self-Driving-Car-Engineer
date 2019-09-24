## 项目介绍
无人车的输入包含一张地图，一个带噪声的GPS获取的初始位置，以及传感器的感知结果和控制量。



项目的流程如下：

![pipeline](./pic/pipeline.png)



## 如何运行代码？

项目包括Term 2的模拟器，你可以在[这里](https://github.com/udacity/self-driving-car-sim/releases)下载

运行代码前需要安装uWebSocketIO库，主程序通过如下方式编译和运行：

1. mkdir build
2. cd build
3. cmake ..
4. make
5. ./particle_filter




## 输入

1. 一张包含路标位置的地图
2. 一个初始的GPS定位结果
3. 无人车移动时对路标的感知结果



## 输出

![pipeline](./pic/result.png)

在图像中，Udacity-car是无人车的真值位置，如下所示：

![pipeline](./pic/GroundTruth_icon.png)

蓝色的圈中包含一个黑色的箭头是算法结果，如下所示：

![pipeline](./pic/ParticlePosition_icon.png)

