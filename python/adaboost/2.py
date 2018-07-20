import math
import time
from numpy import *

start_time = time.time()

f_train = open('train.csv')
f_test = open('test.csv')

train_label = []
test_label = []
train_data = []
test_data = []

for l in f_train:
  line_content = l.split(',')
  train_label.append(int(line_content[0]))
  data = []
  for i in line_content[1:]:
    data.append(float(i))
  train_data.append(data)

for l in f_test:
  line_content = l.split(',')
  test_label.append(int(line_content[0]))
  data = []
  for i in line_content[1:]:
    data.append(int(i))
  test_data.append(data)

'''
train_num_row = len(train_data)
train_num_column = len(train_data[0])
test_num_row = len(test_data)
test_num_column = len(test_data)
'''

train_data = mat(train_data)
test_data = mat(test_data)
train_n_row, train_n_col = shape(train_data)
test_n_row, test_n_col = shape(test_data)

train_label = mat(train_label).T
train_label[train_label != 1] = -1
#print(train_label)

test_label = mat(test_label).T
test_label[test_label != 1] = -1

num_iter = 10
step_gap = 0.01
num_step = 1/step_gap
weight = [1/train_n_row]*train_n_row
weight = mat(weight).T

def PrintClassifier(classifier):
  print('The classifier is:')
  des = ''
  for i in classifier:
    des += str(i[-1]) + '*X' + str(i[0]) + str(i[1]) + str(i[2]) + ' '
  print(des)

def matexp(mat):
  m,n = shape(mat)
  for i in range(m):
    for j in range(n):
      mat[i,j] = exp(mat[i,j])
  return mat

def Generateclassifier(data_mat, label_mat, weight, step_gap=0.1, n_step=100):
  n_row, n_col = shape(data_mat)
  best_err_rate = 1.1
  best_i = -1
  best_value = -1
  best_pred = ones((n_row, 1))
  best_mode = '>'
  for i in range(n_col):
    for j in range(n_step): # traverse all the possible gap_value
      this_v = (j+1)*step_gap
      x = data_mat[:, i]
      for mode in ['>', '<']:
        pred = -ones((n_row, 1))
        if mode == '>':
          pred[x > this_v] = 1  # x > this_v: will be 1
        else:
          pred[x < this_v] = 1
        err_mat = multiply(pred, label_mat) < 0
        err = (err_mat.T*weight)[0,0]
        #print('for ', this_v, ', err is ', err)
        #print('pred is ', pred)
        #print('label_mat is ', label_mat)
        if err < best_err_rate:
          #print('err_mat is ', err_mat)
          best_mode = mode
          best_err_rate = err
          best_i = i
          best_value = this_v
          best_pred = pred
  return best_err_rate, best_mode, best_value, best_i, best_pred

'''
best_err_rate, best_mode, best_value, best_i, best_pred = Generateclassifier(train_data, train_label, weight)
print('best is: ', best_i, best_mode, best_value, best_pred, best_err_rate)
'''
print('before the train, time cost is ', time.time()-start_time)
start_time = time.time()
num_iter = 10
last_pred = zeros((train_n_row, 1))
classifier = []
for i in range(num_iter):
  best_err_rate, best_mode, best_value, best_i, best_pred = Generateclassifier(train_data, train_label, weight)
  alpha = float(0.5*log((1-best_err_rate)/max(best_err_rate, 0.001)))
  #print('best errrate is ', best_err_rate)
  last_pred += alpha * best_pred
  exon = matexp(-alpha*multiply(train_label, best_pred))
  #print('train_label is ', train_label)
  #print('best_pred is ', best_pred)
  #print('mul is ', multiply(train_label, best_pred))
  #print('exon is ', exon)
  weight = multiply(weight, exon)
  weight = weight/weight.sum()
  #print('weight is ', weight)
  err_mat = multiply(last_pred, train_label) < 0
  err_rate = err_mat.mean()
  cl = []
  cl.append(best_i)
  cl.append(best_mode)
  cl.append(best_value)
  cl.append(alpha)
  classifier.append(cl)
  print(i, 'th train, time cost is ', time.time()-start_time)
  start_time = time.time()
  if err_rate < 0.0001:
    break

def Judge(data, label, classifier):
  aggv = 0.0
  labelest = 0
  for i in range(len(classifier)):
    if classifier[i][1] == '>':
      if data[0, classifier[i][0]] > classifier[i][2]:
        aggv += classifier[i][3]
      else:
        aggv -= classifier[i][3]
    else:
      if data[0, classifier[i][0]] < classifier[i][2]:
        aggv += classifier[i][3]
      else:
        aggv -= classifier[i][3]
  if aggv > 0:
    labelest = 1
  else:
    labelest = -1
  return labelest == label

PrintClassifier(classifier)
count = 0
for i in range(len(test_data)):
  is_correct = Judge(test_data[i], test_label[i], classifier)
  count += is_correct
  print('for ', i, 'th data, is_correct is ', is_correct)
print('accurancy is ', count/test_n_row)
print('judge time cost is ', time.time()-start_time)
