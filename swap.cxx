#include <iostream>
#include <algorithm>

void DoStuff(int** arr1, int** arr2)
{
  std::swap(*arr1, *arr2);
}


int main(int argc, char **argv)
{
  int *arr1 = new int[5];//{1,2,3,4,5};
  int *arr2 = new int[5];//{9,10,11,12,13};

  for(int i=0; i < 5; i++)
    arr1[i] = i;
  for(int i=0; i < 5; i++)
    arr2[i] = i + 10;

  std::cout << "Before swapping" << std::endl;
  for(int i=0; i < 5; i++)
    std::cout << arr1[i] << "\t";
  std::cout << std::endl;
  for(int i=0; i < 5; i++)
    std::cout << arr2[i] << "\t";
  std::cout << std::endl;

  DoStuff(&arr1, &arr2);

  std::cout << "After swapping" << std::endl;
  for(int i=0; i < 5; i++)
    std::cout << arr1[i] << "\t";
  std::cout << std::endl;
  for(int i=0; i < 5; i++)
    std::cout << arr2[i] << "\t";
  std::cout << std::endl;
}
