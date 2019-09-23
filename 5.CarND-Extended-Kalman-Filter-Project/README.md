## Extended Kalman Filter Project

### Compile & Run

```
$ mkdir build
$ cd build
$ cmake .. & make
$ ./ExtendedKF
```

### Visualization

Download EKF simulator in websit : [EKF_Simulator](https://github.com/udacity/self-driving-car-sim/releases/), then unzip the file. run the following command:

```
$ cd term2_sim_linux
$ chmod a+x term2_sim.x86_64
$ ./term2_sim.x86_64
```

push button **start** to visual the result.

### Result

![DataSet 1 Result](./readme_pic/dataset1_result.png)

The RMSE result in DataSet 1 is [0.0974, 0.0854, 0.4455, 0.4451].

![DataSet 2 Result](./readme_pic/dataset2_result.png)

The RMSE result in DataSet 2 is [0.0726, 0.0964, 0.4093, 0.4769].
