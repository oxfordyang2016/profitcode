'''
import numpy as np
from keras.models import Sequential
from keras.layers.core import Dense, Activation
from keras.layers import Conv2D, MaxPooling2D, Flatten
from keras.layers import Conv1D, MaxPool1D
from keras.optimizers import SGD, Adam
from keras.utils import np_utils
from keras.datasets import mnist
from keras import backend as K
'''

from keras.optimizers import SGD, Adam
import numpy as np
np.random.seed(1337)  # for reproducibility
from keras.datasets import mnist
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten
from keras.layers import Convolution2D, MaxPooling2D
from keras.utils import np_utils
from keras import backend as K

(X_train, y_train), (X_test, y_test) = mnist.load_data()

X_train = X_train/255
X_test = X_test/255

#xtrain = X_train.reshape(len(X_train), 28*28)
ytrain = np_utils.to_categorical(y_train, 10)

xtest = X_test.reshape(len(X_test), 28*28)
ytest = np_utils.to_categorical(y_test, 10)

img_rows, img_cols = 28, 28
if K.image_dim_ordering() == 'th':  
    X_train = X_train.reshape(X_train.shape[0], 1, img_rows, img_cols)  
    X_test = X_test.reshape(X_test.shape[0], 1, img_rows, img_cols)  
    input_shape = (1, img_rows, img_cols)  
else:  
    X_train = X_train.reshape(X_train.shape[0], img_rows, img_cols, 1)  
    X_test = X_test.reshape(X_test.shape[0], img_rows, img_cols, 1)  
    input_shape = (img_rows, img_cols, 1)  

model = Sequential()

#model.add(Conv1D(25,3,3,input_shape=(28, 28)))
#model.add(MaxPooling1D((2,2)))
#model.add(Dense(input_dim=28*28, units = 600, activation='relu'))
#model.add(Dense(units = 80, activation='relu'))
#model.add(Dense(units = 60, activation='relu'))
#model.add(Dense(units = 50, activation='relu'))


model.add(Convolution2D(25, (3, 3), input_shape=input_shape))
model.add(MaxPooling2D((2,2)))

model.add(Convolution2D(128, (4, 4)))
model.add(MaxPooling2D((2,2)))

model.add(Flatten())

model.add(Dense(units=100))
model.add(Activation('relu'))

model.add(Dense(units=10))
model.add(Activation('softmax'))

#model.add(Dense(units = 10, activation='softmax'))

model.compile(loss = 'categorical_crossentropy', optimizer=Adam(),metrics=['accuracy'])
model.fit(X_train, ytrain, batch_size=1000, epochs=10, verbose=1)

print('train accuracy is ', model.evaluate(X_train, ytrain, batch_size=100)[1])
print('accuracy is ', model.evaluate(X_test, ytest, batch_size=100)[1])
