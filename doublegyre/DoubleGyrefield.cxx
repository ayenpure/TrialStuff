#include <cmath>

#include <vtkm/Types.h>

#define PI 3.14159265
#define PERIOD 10

template <typename VectorType>
class DoubleGyrefield
{
public :

  using ScalarType = typename VectorType::ComponentType;
  static constexpr ScalarType A = 0.1;
  static constexpr ScalarType w = (2*PI)/PERIOD;
  static constexpr ScalarType ep = 0.25;

  static void calculateVelocity(const VectorType& location, vtkm::FloatDefault t, VectorType& velocity)
  {

    ScalarType a_t = ep * sin(w*t);
    ScalarType b_t = 1 - (2 * ep * sin(w*t));

    ScalarType fx = (a_t * (location[0]*location[0])) + (b_t*location[0]);
    ScalarType dfx = (2*a_t*location[0]) + b_t;

    velocity[0] = (-1) * A * sin(PI*fx) * cos(PI * location[1]);
    velocity[1] = PI * A * cos(PI * fx) * sin(PI * location[1]) * dfx;
    velocity[2] = 0;
  }
};
