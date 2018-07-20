#include <iostream>
#include <string>

using namespace std;

void fun(int a[]) {
  a[0] = 1000;
}

int main() {
  int a[100];
  a[0] = 10;
  fun(a);
  cout << a[0];
}
