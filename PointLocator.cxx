namespace vtkm
{

namespace exec
{

class PointLocator
{
  VTKM_EXEC virtual FindNearestNeighbor(vtkm::Vec<vtkm::FloatDefault, 3> queryPoint
                                        vtkm::Id &pointId,
                                        FloatDefault &distance) const = 0;
}

} // namespace exec

namespace cont
{

class PointLocator : public ExecuteObjectBase
{

public:
  PointLocator();

  vtkm::cont::CoordinateSystem GetCoords() const;

  void SetCoords(const vtkm::cont::CoordinateSystem &coords)
  {
    this->coordinates = coords;
  }

  virtual void Build() = 0;

  VTKM_CONT virtual void
  FindNearestNeighbors(const vtkm::cont::ArrayHandleVirtualCoordinates &points,
                       vtkm::cont::ArrayHandle<vtkm::Id> &nearestNeighborIds,
                       vtkm::cont::ArrayHandle<vtkm::FloatDefault> &distances) const
  {
    //Default Implementation
  }

  template<typename Type, typename Storage>
  VTKM_CONT void FindNearestNeighbors(const vtkm::cont::ArrayHandleVirtualCoordinates &points,
                                      vtkm::cont::ArrayHandle<vtkm::Id> &nearestNeighborIds,
                                      vtkm::cont::ArrayHandle<vtkm::FloatDefault> &distances) const
  {
    this->FindCells(vtkm::cont::ArrayHandleVirtualCoordiantes(points),
                    nearestNeighborIds,
                    distances);
  }

  template<typename DeviceAdapter>
  VTKM_CONT vtkm::exec::PointLocator PrepareForExecution(DeviceAdapter device)
  {
    return this->PrepareForExecution(GetDeviceId(device));
  }

  VTKM_CONT virtual vtkm::exec::PointLocator PrepareForExecution(vtkm::Id device) {} = 0;

private:
  vtkm::cont::CoordinateSystem coordinates;
}

} // namespace cont
} // namespace vtkm
