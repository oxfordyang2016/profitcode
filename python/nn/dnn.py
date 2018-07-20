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
from time import *

(X_train, y_train), (X_test, y_test) = mnist.load_data()

X_train = X_train/255
X_test = X_test/255

img_rows, img_cols = 28, 28

X_train = X_train.reshape(X_train.shape[0], 1, img_rows*img_cols)
X_test = X_test.reshape(X_test.shape[0], 1, img_rows*img_cols)

ytrain = np_utils.to_categorical(y_train, 10)
ytest = np_utils.to_categorical(y_test, 10)

ytrain = ytrain.reshape(ytrain.shape[0], 1, ytrain.shape[1])
ytest = ytest.reshape(ytest.shape[0], 1, ytest.shape[1])

input_shape = (1,784)

model = Sequential()

model.add(Dense(units=600, input_shape = input_shape))
model.add(Activation('relu'))

model.add(Dense(units=300, input_shape = input_shape))
model.add(Activation('relu'))
#model.add(Dropout(0.5))

model.add(Dense(units=100, input_shape = input_shape))
model.add(Activation('relu'))

model.add(Dense(units=10))
model.add(Activation('softmax'))

model.compile(loss = 'categorical_crossentropy', optimizer=Adam(),metrics=['accuracy'])
start_time = time()
model.fit(X_train, ytrain, batch_size=1000, epochs=20, verbose=1)
print('training time is ', time()-start_time)

print('train accuracy is ', model.evaluate(X_train, ytrain, batch_size=100)[1])
print('accuracy is ', model.evaluate(X_test, ytest, batch_size=100)[1])
