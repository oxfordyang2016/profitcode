#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#define MAXN 200000

using namespace std;

double gold[MAXN];
double sliver[MAXN];

void loaddata() {
  fstream gold_file;
  gold_file.open("gold.data");
  int count = 0;
  while (!gold_file.eof()) {
    char temp[100];
    gold_file.getline(temp, 100);
    // printf("%s\n", temp);
    gold[count++] = atof(temp);
  }
  count = 0;
  fstream sliver_file;
  sliver_file.open("sliver.data");
  while (!sliver_file.eof()) {
    char temp[100];
    sliver_file.getline(temp, 100);
    // printf("%s\n", temp);
    sliver[count++] = atof(temp);
  }
}

double calcost(double x[], double y[], int size, double k, double b) {
  double cost = 0.0;
  for (int i = 0; i < size; i++) {
    cost += pow((y[i] - k*x[i] - b), 2);
  }
  cost /= 2*size;
  return cost;
}

int main() {
  loaddata();
  // printf("the last price is %lf and %lf\n", gold[9], sliver[9]);
  double initk = 0.0;
  double initb = 0.0;

  double k = 0.0;
  double b = 0.0;

  int iter_nums = 1000;
  double learn_rate = 0.01;

  int size = 19000;

  for (int i = 0; i < iter_nums; i++) {
    double dk = 0.0;
    for (int j = 0; j < size; j++) {
      dk += (gold[j] - k*sliver[j] - b)*sliver[j];
    }
    dk /= size;
    k = k - learn_rate * dk;
    double db = 0.0;
    for (int j = 0; j < size; j++) {
      db += (gold[j] - k*sliver[j] - b);
    }
    db /= size;
    b = b - learn_rate * db;
    printf("%d iterations, cost is %lf\n", i+1, calcost(gold, sliver, size, k, b));
  }
}
