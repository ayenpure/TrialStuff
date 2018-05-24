#include <iostream>

class Base
{
public :
  virtual void printStuff() const
  {
    std::cout << "Rediculous" << std::endl;
  }
};

template<class T>
class Derived : public Base
{
private :
  T value;
public :
  Derived(T val) : value(val)
  {};

  /*void printStuff() const
  {
    std::cout << "Value : " << value<< std::endl;
  }*/
};

int main()
{
  Derived<int> d(100);
  std::cout << "Attempting to print" << std::endl;
  d.printStuff();
  (static_cast<Base>(d)).printStuff();
}
