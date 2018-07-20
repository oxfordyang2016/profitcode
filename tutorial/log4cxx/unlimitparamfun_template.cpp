#include <iostream>
using namespace std;
//递归终止函数
void print()
{
   cout << "empty" << endl;
}
//展开函数
template <class T, class ...Args>
void print(T head, Args... rest)
{
   cout << head;
   print(rest...);
}


int main(void)
{
   print(1,2,3,4, "asd");
   return 0;
}
