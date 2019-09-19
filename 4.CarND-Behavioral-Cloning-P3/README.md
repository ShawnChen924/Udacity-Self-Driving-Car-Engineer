# **Behavioral Cloning** 

#### **Behavioral Cloning Project**

The goals / steps of this project are the following:
* Use the simulator to collect data of good driving behavior
* Build, a convolution neural network in Keras that predicts steering angles from images
* Train and validate the model with a training and validation set
* Test that the model successfully drives around track one without leaving the road
* Summarize the results with a written report



#### Files

**drive.py** is for loading model and driving car in autonomous mode. Default speed : 18km/h

**model.h5** is the model I submit.

**model.py** is the code for training model.

**video.mp4** is the result of autonomous car in track 1.



#### How to run the code

Training the model, run

```
python model.py
```

Using model.h5 to test in Udacity car simulator, run

```
python drive.py model.h5
```



#### Model Architecture

Using keras to express the model architecture is like :

```
input_shape = (160, 320, 3)
ouput_shape = (160, 320, 1)
normal_shape = (80, 320, 1)
model = Sequential()
model.add(Lambda(color2gray, input_shape = input_shape, output_shape= ouput_shape))
model.add(Cropping2D(cropping=((50,30), (0,0))))
model.add(Lambda(lambda x: x/127.5 - 1., output_shape= normal_shape))
model.add(Convolution2D(24,5,5,subsample=(2,2), activation="relu"))
model.add(Convolution2D(36,5,5,subsample=(2,2), activation="relu"))
model.add(Convolution2D(48,5,5,subsample=(2,2), activation="relu"))
model.add(Convolution2D(64,3,3, activation="relu"))
model.add(Convolution2D(64,3,3, activation="relu"))
model.add(Flatten())
model.add(Dropout(0.5)) # reduce overfitting
model.add(Dense(180, activation="relu"))
model.add(Dense(60))
model.add(Dense(10, activation="relu"))
model.add(Dense(1))
```

This model is adapted an End-to-End CNN which released by  Nvidia autonomous vehicle team. I add a dropout layer after flatten layer to reduce overfitting. then add relu to introduce nonlinearity into this model.

|       Layer       |                 Description                  |
| :---------------: | :------------------------------------------: |
|       Input       |        color image, size [160x320x3]         |
|      Lambda       | convert color to grayscale, size [160x320x1] |
|    Cropping2D     |     cropping the image, size [80x320x1]      |
|      Lambda       |      normalization, outputs [80x320x1]       |
| Convolution2D 5x5 |          2x2 stride, valid padding           |
|       RELU        |            introduce nonlinearity            |
| Convolution2D 5x5 |          2x2 stride, valid padding           |
|       RELU        |            introduce nonlinearity            |
| Convolution2D 5x5 |          2x2 stride, valid padding           |
|       RELU        |            introduce nonlinearity            |
| Convolution2D 3x3 |          1x1 stride, valid padding           |
|       RELU        |            introduce nonlinearity            |
| Convolution2D 3x3 |          1x1 stride, valid padding           |
|       RELU        |            introduce nonlinearity            |
|      Flatten      |                                              |
|      Dropout      |            keep probability = 0.5            |
|  Fully connected  |                  output 180                  |
|       RELU        |            introduce nonlinearity            |
|  Fully connected  |                  output 60                   |
|  Fully connected  |                  output 10                   |
|       RELU        |            introduce nonlinearity            |
|  Fully connected  |                   output 1                   |

The input image like:

![color_image](./writeup_pic/1.color_image.jpg)

In this project, color won't make big difference, so I convert color image into gray image, like :

![gray_image](./writeup_pic/2.gray_image.jpg)

The top and bottom part in gray image are irrelevant to the driving behavior, so I cropped them from gray image, like:

![gray_image](./writeup_pic/3.cropped_image.jpg)



#### How to reduce overfitting in the model

1. I collect both clockwise and counter clockwise driving data to avoid overfitting the steering angle to one side.
2. Applying dropout layer in the model to make the network reduce overfitting.
3. Using left and right camera data to augment data set.