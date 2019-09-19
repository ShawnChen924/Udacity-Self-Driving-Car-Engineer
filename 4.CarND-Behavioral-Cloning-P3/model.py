import csv
import cv2
import sklearn
from sklearn.model_selection import train_test_split
from keras.models import Sequential, Model
from keras.layers import Lambda, Input, Flatten, Dense, Cropping2D, Convolution2D, Dropout
import numpy as np

def color2gray(xin):
    return (0.21 * xin[:,:,:,:1]) + (0.72 * xin[:,:,:,1:2]) + (0.07 * xin[:,:,:,-1:])

def generator(dir, samples, batch_size=32):
    num_samples = len(samples)
    image_dir = './' + dir + '/IMG/'
    while 1: 
        sklearn.utils.shuffle(samples)
        for offset in range(0, num_samples, batch_size):
            batch_samples = samples[offset:offset+batch_size]
            images = []
            angles = []
            for batch_sample in batch_samples:
                # center image
                name = image_dir + batch_sample[0].split('/')[-1]
                center_image = cv2.imread(name)
                center_angle = float(batch_sample[3])
                images.append(center_image)
                angles.append(center_angle)
                correction = 0.3
                # left camera image
                left_angle = center_angle + correction
                lname = image_dir + batch_sample[1].split('/')[-1]
                left_image = cv2.imread(lname)
                images.append(left_image)
                angles.append(left_angle)
                # right camera image
                right_angle = center_angle - correction
                rname = image_dir + batch_sample[2].split('/')[-1]
                right_image = cv2.imread(rname)
                images.append(right_image)
                angles.append(right_angle)

            X_train = np.array(images)
            y_train = np.array(angles)
            yield sklearn.utils.shuffle(X_train, y_train)

dirs = ['data_1r1b', 'data_2r1b']

# Set Model
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

# Load Data
for dir in dirs:
    lines = []
    csv_dir = './' + dir + '/driving_log.csv'
    with open(csv_dir) as csvfile:
        reader = csv.reader(csvfile)
        for line in reader:
            lines.append(line)
    train_samples, validation_samples = train_test_split(lines, test_size=0.2)
    print(len(train_samples))
    print(len(validation_samples))

    # Training Model
    train_generator = generator(dir, train_samples, batch_size=32)
    validation_generator = generator(dir, validation_samples, batch_size=32)
    model.compile(loss='mse', optimizer='adam')
    ntrain = len(train_samples)*3
    nvalid = len(validation_samples)*3
    for n_epoch in range(3):
        epochs_num = n_epoch + 1
        history_object = model.fit_generator(train_generator, samples_per_epoch = 
        ntrain, validation_data=validation_generator, 
        nb_val_samples=nvalid, epochs=epochs_num, verbose = 1)
        model_name = 'model_' + dir + '_ep_' + str(epochs_num) + '.h5'
        model.save(model_name)
