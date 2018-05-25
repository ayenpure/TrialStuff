// Type your code here, or load an example.

#include <array>
#include <iostream>
#include <tuple>
#include <utility>


struct SerialType
{
    static constexpr int value = 1;
    constexpr const char* name() const { return "serial"; }
};

struct TBBType
{
    static constexpr int value = 2;
    constexpr const char* name() const { return "tbb"; }
};

struct CudaType
{
    static constexpr int value = 3;
    constexpr const char* name() const { return "cuda"; }
};

using PossibleTypes = std::tuple<SerialType,TBBType,CudaType>;

struct ExampleCallableFunction
{
    template<typename Device>
    void operator()(Device d, std::string message, int times_to_print) const
    {
        std::cout << "runnning on device: " << d.name() << "\n";
        for(int i=0; i < times_to_print; ++i)
        {
            std::cout << message << "\n";
        }
        std::cout << std::endl;
    }
};

template<typename Functor>
struct CallForSpecificDevice
{
    CallForSpecificDevice(const Functor& func): f(func) {}

    template<typename Device, typename... Args>
    void operator()(Device d, int desired_device, Args&&... args) const
    {
        if(d.value == desired_device)
        {
            f(d,std::forward<Args>(args)...);
        }
    }
    const Functor& f;
};

template <typename Functor, typename T1, typename T2, typename T3, typename... Args>
void list_for_each(Functor&& f, std::tuple<T1, T2, T3>, Args&&... args)
{
  f(T1{}, std::forward<Args>(args)...);
  f(T2{}, std::forward<Args>(args)...);
  f(T3{}, std::forward<Args>(args)...);
}

template<typename Types, typename Functor, typename... Args>
void call_function(int desired_device,
                  Types possible_devices,
                  const Functor& functor,
                  Args&&... args)
{
    CallForSpecificDevice<Functor> newFunctor(functor);
    list_for_each(newFunctor, possible_devices, desired_device, std::forward<Args>(args)...);
}


int main(int i, char**)
{

    ExampleCallableFunction function;
    std::string message("hello world");
    int timesToPrintHelloWorld = 5;
    call_function(2, PossibleTypes{}, function, message, timesToPrintHelloWorld );

}
