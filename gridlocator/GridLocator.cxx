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

#include "CellInterpolationHelper.h"

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
  ExecutionGridEvaluator(std::shared_ptr<vtkm::cont::CellLocator> locator,
                         std::shared_ptr<vtkm::cont::CellInterpolationHelper> interpolationHelper)
  {
    Locator = locator->PrepareForExecution(DeviceAdapter());
    InterpolationHelper = interpolationHelper->PrepareForExecution(DeviceAdapter());
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
    vtkm::Id fieldIndices[8];
    vtkm::Id vertices = InterpolationHelper->GetFieldIndices(cellId, fieldIndices);
    vtkm::Id shape = InterpolationHelper->GetCellShape(cellId);
    std::cout << "Number of vertices : " << vertices << std::endl;
    std::cout << "Cell shape : " << (vtkm::Id)shape << std::endl;
  }

private:
  const vtkm::exec::CellLocator* Locator;
  const vtkm::exec::CellInterpolationHelper* InterpolationHelper;
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
    if(cellset.IsSameType(StructuredType()))
    {
      if(coordinates.GetData().IsType<UniformType>())
      {
        vtkm::cont::CellLocatorUniformGrid locator;
        locator.SetCoordinates(coordinates);
        locator.SetCellSet(cellset);
        locator.Update();
        this->Locator = std::make_shared<vtkm::cont::CellLocatorUniformGrid>(locator);
      }
      else if(coordinates.GetData().IsType<RectilinearType>() && cellset.IsSameType(StructuredType()))
      {
        vtkm::cont::CellLocatorRectilinearGrid locator;
        locator.SetCoordinates(coordinates);
        locator.SetCellSet(cellset);
        locator.Update();
        this->Locator = std::make_shared<vtkm::cont::CellLocatorRectilinearGrid>(locator);
      }
      vtkm::cont::StructuredCellInterpolationHelper interpolationHelper(cellset);
      this->InterpolationHelper
        = std::make_shared<vtkm::cont::StructuredCellInterpolationHelper>(interpolationHelper);
    }
  }

  template <typename DeviceAdapter>
  VTKM_CONT ExecutionGridEvaluator<DeviceAdapter> PrepareForExecution(DeviceAdapter) const
  {
    return ExecutionGridEvaluator<DeviceAdapter>(this->Locator, this->InterpolationHelper);
  }

private:
  std::shared_ptr<vtkm::cont::CellLocator> Locator;
  std::shared_ptr<vtkm::cont::CellInterpolationHelper> InterpolationHelper;
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

