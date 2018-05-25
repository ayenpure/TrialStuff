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
public:
  VTKM_EXEC virtual FindCell(vtkm::Vec<vtkm::FloatDefault, 3>& point,
                             vtkm::Id& cellId,
                             vtkm::Vec<vtkm::FloatDefault, 3>& parametric) const = 0;
}

} // namespace exec

namespace cont
{

class CellLocator : public ExecutionObjectBase
{

public:
  CellLocator()
    : dirty(true)
  {
  }

  vtkm::cont::DynamicCellSet GetCellSet() const { return cellSet(); }

  void SetCellSet(const vtkm::cont::DynamicCellSet& cellSet_)
  {
    cellSet = cellSet_;
    dirty = true;
  }

  vtkm::cont::CoordinateSystem GetCoords() const { return coords; }

  void SetCoords(const vtkm::cont::CoordinateSystem& coords_)
  {
    coords = coords_;
    dirty = true;
  }

  virtual void Build() = 0;

  void Update()
  {
    if (dirty)
      Build();
    dirty = false;
  }

  <typename DeviceAdapter> VTKM_CONT vtkm::cont::DeviceAdapterId GetDeviceId(DeviceAdapter device)
  {
    return vtkm::cont::DeviceAdapterTraits<DeviceAdapter>::GetId();
  }

  template<typename DeviceAdapter>
  VTKM_CONT vtkm::exec::CellLocator PrepareForExecution(DeviceAdapter device)
  {
    return PrepareForExecution(GetDeviceId(device));
  }

  VTKM_CONT virtual vtkm::exec::CellLocator PrepareForExecution(
    vtkm::cont::DeviceAdapterId device) = 0;

private:
  vtkm::cont::DynamicCellSet cellSet;
  vtkm::cont::CoordinateSystem coordinates;
  bool dirty;
};

} // namespace cont
} // namespace vtkm
