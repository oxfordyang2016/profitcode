import numpy as np
from random import random
import math

def sigmoid(x):
  return 1.0/ (1.0 + math.exp(-x))

def actfun(data):
  result = [[0 for i in range(len(data[0]))] for j in range(len(data))]
  for i in range(len(data)):
    for j in range(len(data[0])):
      result[i][j] = sigmoid(data[i][j])
  return result

def MatTranpose(list1):
  row, column = np.mat(list1).shape
  result = [[0 for i in range(row)] for j in range(column)]
  for i in range(column):
    for j in range(row):
      result[i][j] = list1[j][i]
  return result

def MatMul(list1, list2):
  if len(list1[0]) != len(list2):
    print('%s\'s dimension is not confront to %s, dimension is %d and %d'%(list1, list2, len(list1[0]), len(list2)))
    return -1
  merge_size = len(list1[0])
  #print('merge size is ', merge_size)
  result = [[0 for i in range(len(list2[0]))] for j in range(len(list1))]
  for i in range(len(list1)):
    for j in range(len(list2[0])):
      for k in range(merge_size):
        result[i][j] += list1[i][k]*list2[k][j]
  return result

def MatAddInt(list1, n):
  result = [[0 for i in range(len(list1[0]))] for j in range(len(list1))]
  for i in range(len(list1)):
    for j in range(len(list1[0])):
      result[i][j] += list1[i][j] + n
  return result

def MatSum(list1):
  result = 0
  for i in range(len(list1)):
    for j in range(len(list1[0])):
      result += list1[i][j]
  return result

def MatAdd(list1, list2):
  row = len(list1)
  column = len(list1[0])
  if row != len(list2) or column != len(list2[0]):
    print 'matadd met wrong size! list1 is '+ str(list1) + ' list2 is ' + str(list2)
    return -1
  result = [[0 for p in range(column)] for q in range(row)]
  for i in range(row):
    for j in range(column):
      result[i][j] = list1[i][j] + list2[i][j]
  return result

def MatMulInt(list1, n):
  row = len(list1)
  column = len(list1[0])
  result = [[0 for p in range(column)] for q in range(row)]
  for i in range(row):
    for j in range(column):
      result[i][j] = list1[i][j] * n
  return result

def MatElementWise(list1, list2):
  row = len(list1)
  column = len(list1[0])
  if row != len(list2) or column != len(list2[0]):
    print 'matadd met wrong size! list1 is '+ list1 + ' list2 is ' + list2
    return -1
  result = [[0 for p in range(column)] for q in range(row)]
  for i in range(row):
    for j in range(column):
      result[i][j] = list1[i][j] * list2[i][j]
  return result

class fullNN:
  def __init__(self, layer_units, input_data, output, learn_rate=0.01, epoches=10):
    self.layer_units = layer_units
    self.learn_rate = learn_rate
    self.layers = len(layer_units)
    self.input_data = input_data
    self.input_data_row = len(input_data)
    self.input_data_column = len(input_data[0])
    self.w = self.InitW()
    self.b = self.InitB()
    self.output = output
    self.z, self.a, self.pred = self.Forward()
    self.theta = self.Backward()
    self.epoches = epoches
    #self.input_data = AddInterception(input_data)

  def AddInterception(input_data):
    return input_data.append(1)

  def InitW(self):
    w = []
    num_loops = self.layers-1
    for i in range(num_loops):
      # fill temp - wight of ith layer, shape should be (lay[i-1], layer[i])
      '''
      if i == 0:  # first hidden layer shape should be (input_column_size, layer[0])
        #temp = [[random() for l in range(self.input_data_column)] for lo in range(self.layer_units[i])]
        temp = [[random() for l in range(self.layer_units[i])] for lo in range(self.input_data_column)]
      else:  # upper layer shape should be (layer[i-1], layer[i])
      '''
      temp = [[random() for l in range(self.layer_units[i+1])] for lo in range(self.layer_units[i])]
      w.append(temp)
    return w

  def InitB(self):
    return [random() for l in range(self.layers-1)]

  def PushInData():
    Forward()
    Backward()

  def PrintForward(self, z, a, pred):
    print '=========================================FORWARD============================================='
    #print 'forward start:'
    print 'input is ' + str(self.input_data)
    print 'weight is ' + str(self.w)
    print 'bias is ' + str(self.b)
    print 'z is ' + str(z)
    print 'a is ' + str(a)
    print 'pred is ' + str(pred)
    print '============================================================================================='

  def PrintBackward(self, theta):  # TODO(nick): here
    print '========================================BACKWARD============================================='
    #print 'forward start:'
    print 'input is ' + str(self.input_data)
    print 'weight is ' + str(self.w)
    print 'bias is ' + str(self.b)
    print 'theta is ' + str(theta)
    print '============================================================================================='

  def Forward(self):
    z = []
    a = []
    num_loops = self.layers-2
    #z.append(input_data*w[0]+b[0])
    #print(self.input_data)
    #print(self.w[0])
    #print(MatMul(self.input_data, self.w[0]))
    z.append(MatAddInt(MatMul(self.input_data, self.w[0]), self.b[0]))
    a.append(actfun(z[-1]))
    for i in range(num_loops):
      #z.append(a[i].w[i+1]+b[i+1])
      #a.append(actfun(z[i+1]))
      z.append(MatAddInt(MatMul(a[i], self.w[i+1]), self.b[i+1]))
      a.append(actfun(z[-1]))
    pred = a[-1]
    #print z
    #print a
    self.PrintForward(z, a, pred)
    return z, a, pred

  def Backward(self):
    theta = []
    theta.append(MatElementWise(MatElementWise((MatAdd(self.pred, MatMulInt(self.output, -1))), self.z[-1]), MatAddInt(MatMulInt(self.z[-1], -1), 1)))
    for i in range(self.layers-2):
      temp = MatMul(theta[-1], MatTranpose(self.w[-i-1]))
      sigmoid_d = MatElementWise(MatAddInt(MatMulInt(self.z[-i-2], -1), 1), self.z[-i-2])
      this_theta = MatElementWise(temp, sigmoid_d)
      theta.append(this_theta)
    theta.reverse()
    self.PrintBackward(theta)
    return theta
  def PrintUpdate(self):
    print '=========================================UPDATE=============================================='
    print 'weight is ' + str(self.w)
    print 'bias is ' + str(self.b)
    print '============================================================================================='
   
  def update(self):
    num_loops = self.layers-1
    for i in range(num_loops):
      self.b[i] = self.b[i] - MatSum(MatMulInt(self.theta[i], self.learn_rate))
      if i == 0:
        #print(MatMulInt(MatMul(MatTranpose(self.input_data), self.theta[i]), self.learn_rate))
        #print(self.input_data)
        #print(self.theta[i])
        self.w[i] = MatAdd(MatMulInt(MatMulInt(MatMul(MatTranpose(self.input_data), self.theta[i]), self.learn_rate), -1), self.w[i])
      else:
        self.w[i] = MatAdd(MatMulInt(MatMulInt(MatMul(MatTranpose(self.a[i-1]), self.theta[i]), self.learn_rate), -1), self.w[i])
    self.PrintUpdate()
  def Calcost(self):
    gap_mat = MatAdd(self.output, MatAddInt(self.pred, -1))
    square = MatElementWise(gap_mat, gap_mat)
    return MatSum(square)
  def run(self):
    for i in range(self.epoches):
      self.z, self.a, self.pred = self.Forward()
      self.theta = self.Backward()
      self.update()
      print 'loss is ' + str(self.Calcost())

dataset = [[2.7810836,2.550537003],
           [3.396561688,4.400293529],
	   [1.38807019,1.850220317],
	   [3.06407232,3.005305973],
	   [7.627531214,2.759262235],
	   [5.332441248,2.088626775],
	   [6.922596716,1.77106367]]
datalabel = [[0],
           [0],
	   [0],
	   [0],
	   [1],
	   [1],
	   [1]]

test_data = [[1.465489372,2.362125076,0],
	     [8.675418651,-0.242068655,1],
	     [7.673756466,3.508563011,1]]
test_data_label = [[0],[1],[1]]

#fn = fullNN([2,2,1], [[0.5,0.5], [0.3, 0.7]], [[1], [0]], learn_rate=0.1)
epoches = 10
fn = fullNN([2,2,1], dataset, datalabel, learn_rate=0.01)
fn.run()
