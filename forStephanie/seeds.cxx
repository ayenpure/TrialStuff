#include <vector>
#include <string>

#include <vtkm/Types.h>
#include <vtkm/cont/DataSet.h>

const vtkm::Id SPARSE = 0;
const vtkm::Id DENSE = 1;
const vtkm::Id MEDIUM = 2;

template <typename T>
static vtkm::Range subRange(vtkm::Range& range, T a, T b)
{
  vtkm::Float32 arg1, arg2, len;
  arg1 = static_cast<vtkm::Float32>(a);
  arg2 = static_cast<vtkm::Float32>(b);
  len = static_cast<vtkm::Float32>(range.Length());
  return vtkm::Range(range.Min + arg1 * len, range.Min + arg2 * len);
}

template<typename FieldType>
void GenerateSeeds(std::vector<vtkm::Vec<FieldType,3>> seeds,
                   vtkm::cont::DataSet& dataset,
                   vtkm::Id numSeeds,
                   std::string fname)
{
  vtkm::Id seeding = DENSE;
  srand(314);
  vtkm::Bounds bounds = dataset.GetCoordinateSystem().GetBounds();
  if (seeding == SPARSE)
    bounds = dataset.GetCoordinateSystem().GetBounds();
  else if (seeding == DENSE)
  {
    if (fname.find("astro") != std::string::npos)
    {
      bounds.X = subRange(bounds.X, .1, .15);
      bounds.Y = subRange(bounds.Y, .1, .15);
      bounds.Z = subRange(bounds.Z, .1, .15);
    }
    else if (fname.find("fusion") != std::string::npos)
    {
      bounds.X = subRange(bounds.X, .8, .85);
      bounds.Y = subRange(bounds.Y, .55, .60);
      bounds.Z = subRange(bounds.Z, .55, .60);
    }
    else if (fname.find("fishtank") != std::string::npos)
    {
      bounds.X = subRange(bounds.X, .1, .15);
      bounds.Y = subRange(bounds.Y, .1, .15);
      bounds.Z = subRange(bounds.Z, .55, .60);
    }
  }
  else if (seeding == MEDIUM)
  {
    if (fname.find("astro") != std::string::npos)
    {
      bounds.X = subRange(bounds.X, .4, .6);
      bounds.Y = subRange(bounds.Y, .4, .6);
      bounds.Z = subRange(bounds.Z, .4, .6);
    }
    else if (fname.find("fusion") != std::string::npos)
    {
      bounds.X = subRange(bounds.X, .01, .99);
      bounds.Y = subRange(bounds.Y, .01, .99);
      bounds.Z = subRange(bounds.Z, .45, .55);
    }
    else if (fname.find("fishtank") != std::string::npos)
    {
      bounds.X = subRange(bounds.X, .4, .6);
      bounds.Y = subRange(bounds.Y, .4, .6);
      bounds.Z = subRange(bounds.Z, .4, .6);
    }
  }

  for (int i = 0; i < numSeeds; i++)
  {
    vtkm::Vec<FieldType, 3> point;
    vtkm::Float32 rx = (vtkm::Float32)rand() / (vtkm::Float32)RAND_MAX;
    vtkm::Float32 ry = (vtkm::Float32)rand() / (vtkm::Float32)RAND_MAX;
    vtkm::Float32 rz = (vtkm::Float32)rand() / (vtkm::Float32)RAND_MAX;
    point[0] = static_cast<FieldType>(bounds.X.Min + rx * bounds.X.Length());
    point[1] = static_cast<FieldType>(bounds.Y.Min + ry * bounds.Y.Length());
    point[2] = static_cast<FieldType>(bounds.Z.Min + rz * bounds.Z.Length());
    seeds.push_back(point);
  }
}
