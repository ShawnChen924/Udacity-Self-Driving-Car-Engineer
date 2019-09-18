# **高级车道线检测** 

### 流程

流程分为7步，如下所示

#### Step 1 . 计算相机的畸变参数

使用 opencv-python 工具标定摄像头

![9](.\camera_cal_output\9.jpg)

#### Step 2 : 图像去畸变

使用标定参数去除图像畸变

![1.test5](.\output_images\1.test5.jpg)

#### Step 3 : 实现逆投影变换

将透视图逆投影为鸟瞰图

![3.test5](.\output_images\3.test5.jpg)

#### Step 4 : 创建二值图像

使用 hlsSSelect() 函数和 labBSelect() 函数筛选出一个包含白色和黄色的二值图

![3.test5](.\output_images\4.test5.jpg)

#### Step 5 : 通过滑动窗口实现车道线

使用滑动窗口检测车道线待选点

![5.test5](.\output_images\5.test5.jpg)

#### Step 6 : 基于最近一次的结果进行车道线跟踪

检测到车道先后，使用上次的曲线方程进行车道线跟踪

![5.test5](.\output_images\6.test5.jpg)

#### Step 7 : 计算真实世界中的道路曲率



#### Step 8 : 将车道线结果绘制在图像上

![7.test5](.\output_images\7.test5.jpg)



项目详细的中文描述及解析见《[再识图像之高级车道线检测](https://zhuanlan.zhihu.com/p/54866418)》