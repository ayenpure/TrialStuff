#include <vtkm/Types.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/ExecutionObjectBase.h>

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
  VTKM_EXEC virtual FindCell(const vtkm::Vec<vtkm::FloatDefault, 3>& point,
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
    : Dirty(true)
  {
  }

  vtkm::cont::DynamicCellSet GetCellSet() const { return CellSet(); }

  void SetCellSet(const vtkm::cont::DynamicCellSet& cellSet)
  {
    CellSet = cellSet;
    Dirty = true;
  }

  vtkm::cont::CoordinateSystem GetCoords() const { return Coords; }

  void SetCoords(const vtkm::cont::CoordinateSystem& coords)
  {
    Coords = coords;
    Dirty = true;
  }

  virtual void Build() = 0;

  void Update()
  {
    if (Dirty)
      Build();
    Dirty = false;
  }

  template<typename DeviceAdapter>
  VTKM_CONT vtkm::exec::CellLocator PrepareForExecution(DeviceAdapter device)
  {
    vtkm::cont::DeviceAdapterId deviceId = vtkm::cont::DeviceAdapterTraits<DeviceAdapter>::GetId();
    return PrepareForExecution(deviceId);
  }

  VTKM_CONT virtual vtkm::exec::CellLocator PrepareForExecution(
    vtkm::cont::DeviceAdapterId device) = 0;

private:
  vtkm::cont::DynamicCellSet CellSet;
  vtkm::cont::CoordinateSystem Coords;
  bool Dirty;
};

} // namespace cont
} // namespace vtkm
