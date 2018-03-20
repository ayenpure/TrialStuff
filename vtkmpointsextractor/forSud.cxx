#include <vector>

#include <vtkm/Types.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetBuilderUniform.h>
#include <vtkm/cont/DataSetFieldAdd.h>
#include <vtkm/filter/Streamline.h>
#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

class GetStreamLineEndPoints : public vtkm::worklet::WorkletMapPointToCell {
public:
  typedef void ControlSignature(CellSetIn, WholeArrayIn<>, FieldOut<>);
  typedef void ExecutionSignature(CellShape, PointCount, PointIndices, _2, _3);

  template <typename CellShapeTag, typename PointCountType,
            typename PointIndicesType, typename CoordinatesPortalType,
            typename PointType>
  VTKM_EXEC void operator()(CellShapeTag shape, PointCountType pointCount,
                            const PointIndicesType &pointIndices,
                            const CoordinatesPortalType &coordsPortal,
                            PointType &point) const {
    point = coordsPortal.Get(pointIndices[pointCount - 1]);
  }
};

int main(int argc, char **argv) {
  // Generate the seed array.
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3>> seedsArrayHandle;
  std::vector<vtkm::Vec<vtkm::Float32, 3>> seeds;
  seeds.push_back(vtkm::Vec<vtkm::Float32, 3>(0.5f, 0.5f, 0.5f));
  seeds.push_back(vtkm::Vec<vtkm::Float32, 3>(3.5f, 3.5f, 3.5f));

  seedsArrayHandle = vtkm::cont::make_ArrayHandle(seeds);

  // Create the dataset
  const vtkm::Id3 dims(5, 5, 5);
  vtkm::Id numPoints = dims[0] * dims[1] * dims[2];
  std::vector<vtkm::Vec<vtkm::Float32, 3>> vectorField(
      static_cast<std::size_t>(numPoints));
  for (std::size_t i = 0; i < static_cast<std::size_t>(numPoints); i++)
    vectorField[i] = vtkm::Vec<vtkm::FloatDefault, 3>(1, 0, 0);
  vtkm::cont::DataSetBuilderUniform dataSetBuilder;
  vtkm::cont::DataSetFieldAdd dataSetField;
  vtkm::cont::DataSet dataSet = dataSetBuilder.Create(dims);
  dataSetField.AddPointField(dataSet, "vector", vectorField);

  // Call the filter
  vtkm::filter::Streamline streamline;
  vtkm::filter::Result result;

  streamline.SetStepSize(0.1);
  streamline.SetNumberOfSteps(20);
  streamline.SetSeeds(seedsArrayHandle);

  vtkm::cont::Field vecField = dataSet.GetField("vector");
  result = streamline.Execute(dataSet, vecField);

  // Extract DataSet from the result.
  vtkm::cont::DataSet output = result.GetDataSet();
  // Write a worklet to extract points from the polylines(CellSetExplicit)
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3>> endPoints;
  using DeviceAdapterTag = VTKM_DEFAULT_DEVICE_ADAPTER_TAG;
  vtkm::worklet::DispatcherMapTopology<GetStreamLineEndPoints,
                                       DeviceAdapterTag>()
      .Invoke(output.GetCellSet(0), output.GetCoordinateSystem(), endPoints);

  auto portal = endPoints.GetPortalConstControl();

  for (int i = 0; i < portal.GetNumberOfValues(); i++) {
    auto endPoint = portal.Get(i);
    std::cout << "{" << endPoint[0] << ", " << endPoint[1] << ", "
              << endPoint[2] << "}" << std::endl;
  }
  return 0;
}
