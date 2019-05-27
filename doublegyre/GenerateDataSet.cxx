#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleUniformPointCoordinates.h>
#include <vtkm/cont/DataSetFieldAdd.h>

#include <vtkm/io/writer/VTKDataSetWriter.h>

#include <vtkm/worklet/DispatcherMapField.h>

#include "DoubleGyrefield.cxx"

namespace
{

class PopulateDoubleGyre : public vtkm::worklet::WorkletMapField
{
public:
  VTKM_EXEC_CONT
  PopulateDoubleGyre() = default;

  using ControlSignature = void(FieldIn, FieldOut);

  using ExecutionSignature = void(_1, _2);

  template <typename VectorType>
  VTKM_EXEC
  void operator()(const VectorType& location,
                  VectorType& output) const
  {
    DoubleGyrefield<VectorType>::calculateVelocity(location, 0.0f, output);
  }
};

}


int main(int argc, char** argv)
{
  using VectorType = vtkm::Vec<vtkm::FloatDefault, 3>;

  vtkm::Id3 dimensions(20, 10, 1);
  VectorType origin(0., 0., 0.);
  VectorType spacing(1./20. , 1./10. , 1.);

  vtkm::cont::DataSet dataset;

  vtkm::cont::ArrayHandleUniformPointCoordinates coordinates(dimensions, origin, spacing);
  dataset.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coordinates", coordinates));

  vtkm::cont::CellSetStructured<3> cellset("cell set");
  cellset.SetPointDimensions(dimensions);
  dataset.AddCellSet(cellset);

  vtkm::cont::ArrayHandle<VectorType> vectorField;
  vtkm::worklet::DispatcherMapField<PopulateDoubleGyre> dispatcher;
  dispatcher.Invoke(coordinates, vectorField);

  vtkm::cont::DataSetFieldAdd fieldAdder;
  fieldAdder.AddPointField(dataset, "velocity", vectorField);

  vtkm::io::writer::VTKDataSetWriter writer("doublegyre.vtk");
  writer.WriteDataSet(dataset);

  return 0;
}
