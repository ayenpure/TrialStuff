namespace vtkm
{

namespace exec
{
// This will actually be used in the Execution Environment.
// As this object is returned by the PrepareForExecution on
// the CellLocator we need it to be covarient, and this acts
// like a base class.
class CellLocator
{
  VTKM_EXEC virtual FindCell(vtkm::Vec<vtkm::FloatDefault, 3> &point,
                             vtkm::Id &cellId,
                             vtkm::Vec<vtkm::FlaotDefault, 3> &paramertic) const = 0;
}

} // namespace exec

namespace cont {

class CellLocatorWorklet : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<Vec3> points,
                                FieldOut<Id> cellIds,
                                FieldOut<Vec3> parametricCoords,
                                ExecObject cellLocator);

  typedef void ExecutionSignature(_1, _2, _3, _4);

  template<PointType, ParametricType, CellLocatorType>
  VTKM_EXEC void oparator()(const PointType &input,
                            vtkm::Id &cellId,
                            ParametricType &paramerticOutput,
                            CellLocatorType &cellLocator)
  {
    cellLocator.FindCell(input, cellId, parametricOutput);
  }
}

class CellLocator : public ExecutionObjectBase
{

public:
  CellLocator();

  vtkm::cont::DynamicCellSet GetCellSet() const
  {
    return this->cellSet();
  }

  void SetCellSet(const vtkm::cont::DynamicCellSet &cellSet_)
  {
    this->cellSet = cellSet_;
    this->dirty = true;
  }

  vtkm::cont::CoordinateSystem GetCoords() const
  {
    return this->coords;
    this->dirty = true;
  }

  void SetCoords(const vtkm::cont::CoordinateSystem &coords_)
  {
    return this->coords = coords_;
  }

  // The following methods should be available as a general VTK-m utilities.
  // Is it already?
  <typename DeviceAdapter>
  VTKM_CONT vtkm::Id GetDeviceId(DeviceAdapter device)
  {
    // Get the device Id from the DeviceAdapter
    // This can be done how Rob has pointed out on the discussion
  }

  //Clean the dirty flag after Building.
  virtual void Build() = 0;

  template<typename DeviceAdapter>
  VTKM_CONT void FindCells(const vtkm::cont::ArrayHandleVirtualCoordinates &points,
                           vtkm::cont::ArrayHandle<vtkm::Id> &cellIds,
                           vtkm::cont::ArrayHandle<vtkm::Vec<vktm::FloatDefault, 3>> &parametricCoords,
                           DeviceAdapter device) const
  {
    // Invoke the worklet with the provided parameters and 'this' as an argument.
    // The PrepareForExecution is expected to be called on 'this' to get the ExecObject
    // into the Worklet.
    if(dirty)
      Build();
    vtkm::worklet::DispatcherMapField<CellLocatorWorklet, DeviceAdapter>().Invoke(
      points,
      cellIds,
      parametricCoords,
      this->PrepareForExecution(device));
  }

  template<typename Type, typename Storage, typename DeviceAdapter>
  VTKM_CONT void FindCells(const vtkm::cont::ArrayHandle<vtkm::Vec<Type, 3>, Storage>& points,
                           vtkm::cont::ArrayHandle<vtkm::Id>& cellIds,
                           vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault, 3>>& parametricCoords,
                           DeviceAdapter device) const
  {
    this->FindCells(vtkm::cont::ArrayHandleVirtualCoordiantes(points), cellIds, parametricCoords, DeviceAdapter);
  }

  template<typename DeviceAdapter>
  VTKM_CONT vtkm::exec::CellLocator PrepareForExecution(DeviceAdapter device)
  {
    //Get the device Id using the DeviceAdapter object
    return this->PrepareForExecution(GetDeviceId(device));
  }

  VTKM_CONT virtual vtkm::exec::CellLocator PrepareForExecution(vtkm::Id device) {} = 0;

private:
  vtkm::cont::DynamicCellSet cellSet;
  vtkm::cont::CoordinateSystem coordinates;
  bool dirty;
};

} // namespace cont
} // namespace vtkm
