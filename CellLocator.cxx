namespace vtkm {

namespace exec
{
// This will actually be used in the Execution Environment.
// As this object is returned by the PrepareForExecution on
// the CellLocator we need it to be covarient, and this acts
// like a base class.
// TODO : Does this need to be templated on Device?
//        Because the derived class might need to store the types
//        of the portal.
class CellLocator
{
  VTKM_EXEC virtual FindCell(vtkm::Vec<vtkm::FloatDefault, 3> &point,
                             vtkm::Id &cellId,
                             vtkm::Vec<vtkm::FlaotDefault, 3> &paramertic) const = 0;
}

} // namespace exec

namespace cont {

class CellLocator : public ExecutionObjectBase {
public:
  CellLocator();

  vtkm::cont::DynamicCellSet GetCellSet() const;

  void SetCellSet(const vtkm::cont::DynamicCellSet &cellSet);

  vtkm::cont::CoordinateSystem GetCoords() const;

  void SetCoords(const vtkm::cont::CoordinateSystem &coords);

  // The following methods should be available as a general VTK-m utilities.
  // Is it already?
  <typename DeviceAdapter>
  VTKM_CONT vtkm::Id GetDeviceId(DeviceAdapter device)
  {
    // Get the device Id from the DeviceAdapter
    // This can be done how Rob has pointed out on the discussion
  }

  virtual void Build() = 0;

  VTKM_CONT virtual void
  FindCells(const vtkm::cont::ArrayHandleVirtualCoordinates &points,
            vtkm::cont::ArrayHandle<vtkm::Id> &cellIds,
            vtkm::cont::ArrayHandle<vtkm::Vec<vktm::FloatDefault, 3>> &parametricCoords) const
  {
    // Invoke the worklet with the provided parameters and 'this' as an argument.
    // The PrepareForExecution is expected to be called on 'this' to get the ExecObject
    // into the Worklet.
  }

  template<typename Type, typename Storage>
  VTKM_CONT void FindCells(const vtkm::cont::ArrayHandle<vtkm::Vec<Type, 3>, Storage>& points,
                           vtkm::cont::ArrayHandle<vtkm::Id>& cellIds,
                           vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault, 3>>& parametricCoords) const
  {
    this->FindCells(vtkm::cont::ArrayHandleVirtualCoordiantes(points), cellIds, parametricCoords);
  }


  // TODO : Figure out the proper return type. Since we have a different kind
  //        of object returned from every Dericed CellLocator, this type must
  //        be covariant.
  <typename DeviceAdapter>
  VTKM_CONT vtkm::exec::CellLocator PrepareForExecution(DeviceAdapter device)
  {
    //Get the device Id using the DeviceAdapter object
    return this->PrepareForExecution(GetDeviceId(device));
  }

  // TODO : This has to return an object which has been
  //        prepared for the provided device id.
  //        How to generalize the returned object for the Decive using this virtual method.
  VTKM_CONT virtual vtkm::exec::CellLocator PrepareForExecution(vtkm::Id device) {} = 0;

private:
  vtkm::cont::DynamicCellSet cellSet;
  vtkm::cont::CoordinateSystem coordinates;
};

} // namespace cont
} // namespace vtkm
