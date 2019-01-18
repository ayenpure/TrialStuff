#include <vtkm/Types.h>

#include <vtkm/cont/CellLocator.h>
#include <vtkm/cont/CellLocatorUniformGrid.h>
#include <vtkm/cont/CellLocatorRectilinearGrid.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/ExecutionObjectBase.h>

#include <vtkm/exec/CellLocator.h>

#include <vtkm/io/reader/VTKDataSetReader.h>

#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/DispatcherMapField.h>

class TrialWorklet : public vtkm::worklet::WorkletMapField
{
public:
  VTKM_CONT
  TrialWorklet() = default;

  using ControlSignature = void(FieldIn point, ExecObject evaluator);

  using ExecutionSignature = void(WorkIndex, _1, _2);

  template <typename PointType,
            typename GridEvaluatorType>
  VTKM_EXEC
  void operator()(const vtkm::Id workIndex,
                  const PointType point,
                  const GridEvaluatorType evaluator) const
  {
    std::cout << "Processing point at index : " << workIndex << std::endl;
    evaluator.Locate(point, (*this));
  }
};

template <typename DeviceAdapter>
class ExecutionGridEvaluator
{
public:
  VTKM_CONT
  ExecutionGridEvaluator() = default;

  VTKM_CONT
  ExecutionGridEvaluator(const vtkm::cont::CellLocator* const* locator)
  {
    std::cout << "Into the Constructor" << std::endl;
    vtkm::cont::CoordinateSystem coords = (*locator)->GetCoordinates();
    std::cout << "Got coordinates" << std::endl;
    vtkm::Bounds bounds = coords.GetBounds();
    std::cout << "Bounds details : " << std::endl;
    std::cout << "X : " << bounds.X.Min << " to " << bounds.X.Max << std::endl;
    std::cout << "Y : " << bounds.Y.Min << " to " << bounds.Y.Max << std::endl;
    std::cout << "z : " << bounds.Z.Min << " to " << bounds.Z.Max << std::endl;
    Locator = (*locator)->PrepareForExecution(DeviceAdapter());
  }

  template <typename Point>
  VTKM_EXEC void Locate(const Point point,
                        const vtkm::exec::FunctorBase& worklet) const
  {
    std::cout << "Finding cell" << std::endl;
    vtkm::Id cellId;
    Point parametric;
    Locator->FindCell(point, cellId, parametric, worklet);
    std::cout << "Cell Id : " << cellId << ", Parametric : " << parametric << std::endl;
  }

private:
  const vtkm::exec::CellLocator* Locator;
  // Add Execution Field here.
};

class GridEvaluator : public vtkm::cont::ExecutionObjectBase
{
public:
  using UniformType = vtkm::cont::ArrayHandleUniformPointCoordinates;
  using AxisHandle = vtkm::cont::ArrayHandle<vtkm::FloatDefault>;
  using RectilinearType = vtkm::cont::ArrayHandleCartesianProduct<AxisHandle, AxisHandle, AxisHandle>;
  using StructuredType = vtkm::cont::CellSetStructured<3>;

  VTKM_CONT
  GridEvaluator(vtkm::cont::CoordinateSystem& coordinates,
                vtkm::cont::DynamicCellSet& cellset)
  {
    //vtkm::cont::CoordinateSystem coords = (*locator)->GetCoordinates();
    vtkm::Bounds bounds = coordinates.GetBounds();
    std::cout << "Bounds details : " << std::endl;
    std::cout << "X : " << bounds.X.Min << " to " << bounds.X.Max << std::endl;
    std::cout << "Y : " << bounds.Y.Min << " to " << bounds.Y.Max << std::endl;
    std::cout << "z : " << bounds.Z.Min << " to " << bounds.Z.Max << std::endl;

    if(coordinates.GetData().IsType<UniformType>() && cellset.IsSameType(StructuredType()))
    {
      std::cout << "Building Uniform" << std::endl;
      vtkm::cont::CellLocatorUniformGrid locator;
      locator.SetCoordinates(coordinates);
      locator.SetCellSet(cellset);
      locator.Update();
      this->Locator = &locator;
      std::cout << "Finished Building Uniform" << std::endl;
    }
    else if(coordinates.GetData().IsType<RectilinearType>() && cellset.IsSameType(StructuredType()))
    {
      std::cout << "Building Rectilinear" << std::endl;
      vtkm::cont::CellLocatorRectilinearGrid locator;
      locator.SetCoordinates(coordinates);
      locator.SetCellSet(cellset);
      locator.Update();
      this->Locator = &locator;
      std::cout << "Finished Building Rectilinear" << std::endl;
    }
    else
    {
      std::cout << "This bitch is explicit." << std::endl;
    }
  }

  template <typename DeviceAdapter>
  VTKM_CONT ExecutionGridEvaluator<DeviceAdapter> PrepareForExecution(DeviceAdapter) const
  {
    std::cout << "Building Execution Grid Evaluator" << std::endl;
    return ExecutionGridEvaluator<DeviceAdapter>(&this->Locator);
  }

private:
  const vtkm::cont::CellLocator* Locator;
  // Add Field here.
};

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    std::cout << "Usage : ./trial <VTK dataset>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string dataset_name(argv[1]);

  vtkm::io::reader::VTKDataSetReader reader(dataset_name);
  vtkm::cont::DataSet dataset = reader.ReadDataSet();

  vtkm::cont::CoordinateSystem coordinates = dataset.GetCoordinateSystem();
  vtkm::cont::DynamicCellSet cellset = dataset.GetCellSet();

  GridEvaluator evaluator(coordinates, cellset);

  using PointType = vtkm::Vec<vtkm::FloatDefault, 3>;
  std::vector<PointType> lookupPoints;
  // Push Sample Points
  lookupPoints.push_back({0, 0.5 ,0});
  lookupPoints.push_back({1, 1, 1});
  lookupPoints.push_back({0, 1, 1.5});
  lookupPoints.push_back({0, 0, 0});
  vtkm::cont::ArrayHandle<PointType> points = vtkm::cont::make_ArrayHandle(lookupPoints);

  TrialWorklet worklet;
  vtkm::worklet::DispatcherMapField<TrialWorklet> dispatcher(worklet);
  dispatcher.Invoke(points, evaluator);
}

