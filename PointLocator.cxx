namespace vtkm
{

namespace exec
{

class PointLocator
{
public:
  VTKM_EXEC virtual FindNearestNeighbor(vtkm::Vec<vtkm::FloatDefault, 3> queryPoint,
                                        vtkm::Id& pointId,
                                        FloatDefault& distanceSquared) const = 0;
}

} // namespace exec

namespace cont
{

class PointLocator : public ExecuteObjectBase
{

public:
  PointLocator()
    : dirty(true)
  {
  }

  vtkm::cont::CoordinateSystem GetCoords() const { return coordinates; }

  void SetCoords(const vtkm::cont::CoordinateSystem& coords)
  {
    coordinates = coords;
    dirty = true;
  }

  virtual void Build() = 0;

  void Update()
  {
    if (dirty)
      Build();
    dirty = false;
  }

  template<typename DeviceAdapter>
  VTKM_CONT vtkm::exec::PointLocator PrepareForExecution(DeviceAdapter device)
  {
    return PrepareForExecution(GetDeviceId(device));
  }

  VTKM_CONT virtual vtkm::exec::PointLocator PrepareForExecution(
    vtkm::cont::DeviceAdapterId device) = 0;

private:
  vtkm::cont::CoordinateSystem coordinates;
  bool dirty;
}

} // namespace cont
} // namespace vtkm
