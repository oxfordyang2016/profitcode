from numpy import *
from time import *
import keras
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten, Convolution2D, MaxPooling2D
from keras.optimizers import Adam
from keras.utils import np_utils
from keras.layers.normalization import BatchNormalization

fp = open('train.csv')
fp_test = open('test.csv')

train_label = []
train_data = []
for line in fp:
  if line[0] == 'l':
    continue
  content = line.split(',')
  train_label.append(int(content[0]))
  train_data.append([int(i) for i in content[1:]])


test_data = []
for line in fp_test:
  if line[0] == 'p':
    continue
  content = line.split(',')
  test_data.append([int(i) for i in content[0:]])

 
train_data_mat = mat(train_data)/255
train_label_mat = mat(train_label).T
train_data_mat = array(train_data_mat).reshape(train_data_mat.shape[0], 28, 28, 1)
print(train_data_mat.shape)
print(train_label_mat.shape)

test_data_mat = mat(test_data)/255
test_data_mat = array(test_data_mat).reshape(test_data_mat.shape[0], 28, 28, 1)

train_data_shape = train_data_mat[0].shape
train_y = np_utils.to_categorical(train_label_mat, 10)

model = Sequential()
model.add(Convolution2D(25, (3,3), input_shape = train_data_shape))
model.add(MaxPooling2D((2,2)))
model.add(BatchNormalization())

model.add(Flatten())

model.add(Dense(units=100))
model.add(Activation('relu'))
model.add(BatchNormalization())

model.add(Dense(units=10))
model.add(Activation('softmax'))

model.compile(loss='categorical_crossentropy', optimizer=Adam(), metrics=['accuracy'])

start_time = time()
model.fit(train_data_mat, train_y, epochs=50, batch_size=1000)
print('time cost is ', time()-start_time)


pred = model.predict(test_data_mat)

f_out = open('pred.csv', 'w')
header = "ImageId,Label\n"
f_out.write(header)

count = 1
for t in pred:
  s = str(count) + ',' + str(argmax(t)) + '\n'
  count += 1
  f_out.write(s)

f_out.close()
fp.close()
