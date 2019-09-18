# **Advanced Lane Lines Detection** 

### Pipeline

My pipeline consisted of 7 steps. They are as follows.

#### Step 1 . Calculate camera distortion coefficients

Using opencv-python tool to calibrate camera

![9](.\camera_cal_output\9.jpg)

#### Step 2 : Undistort image

Using calibration parameters to undistort image of real world

![1.test5](.\output_images\1.test5.jpg)

#### Step 3 : Warp image based on src_points and dst_points

Using fixed points to warp image to bird-view image

![3.test5](.\output_images\3.test5.jpg)

#### Step 4 : Create a thresholded binary image

Using hlsSSelect() method and labBSelect() method to create a thresholded binary image which includes white line and yellow line.

![3.test5](.\output_images\4.test5.jpg)

#### Step 5 : Detect lane lines through moving window

Using moving window method to detect lane line candidate points

![5.test5](.\output_images\5.test5.jpg)

#### Step 6 : Track lane lines based latest lane line result

After lane lines detected, using the equation to track candidate points in next frame image 

![5.test5](.\output_images\6.test5.jpg)

#### Step 7 : Measure real world curvature

Assuming ym_per_pix & xm_per_pix to calculate the curvature and offset of left lane line and right lane line

#### Step 8 : Draw lane line result on undistorted image

![7.test5](.\output_images\7.test5.jpg)



### Reflection

### 1. Identify potential shortcomings with your current pipeline

One potential shortcoming would fail to detect lines when other car cross line driving.

Another shortcoming is that lighting condition may cause false detection result.

### 2. Suggest possible improvements to your pipeline

A possible improvement would be to try to smooth the output through save history detection result

Another potential improvement could be to modify the parameters of binary threshold value.