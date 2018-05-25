namespace vtkm
{

namespace exec
{

class PointLocator
{
public:
  VTKM_EXEC virtual FindNearestNeighbor(vtkm::Vec<vtkm::FloatDefault, 3> queryPoint
                                        vtkm::Id &pointId,
                                        FloatDefault &distance) const = 0;
}

} // namespace exec

namespace cont
{

class PointLocatorWorklet : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<Vec3> point,
                                FieldOut<Id> nearestNeighborId,
                                FieldOut<Float> distance,
                                ExecObject pointLocator);

  typedef void ExecutionSignature(_1, _2, _3, _4);

  template<PointType, PointLocatorType>
  VTKM_EXEC void operator()(const PointType &input,
                            vtkm::Id &nearestNeighborId
                            vtkm::FloatDefault& distance
                            PointLocatorType &pointLocatori)
  {
    pointLocator.FindNearestNeighbor(input, nearestNeighborId, distance);
  }
}

class PointLocator : public ExecuteObjectBase
{

public:
  PointLocator();

  vtkm::cont::CoordinateSystem GetCoords() const
  {
    return this->coordinates;
  }

  void SetCoords(const vtkm::cont::CoordinateSystem &coords)
  {
    this->coordinates = coords;
    this->dirty = true;
  }

  //Clean the ditry flag after building
  virtual void Build() = 0;

  template<typename DeviceAdapter>
  VTKM_CONT void
  FindNearestNeighbors(const vtkm::cont::ArrayHandleVirtualCoordinates &points,
                       vtkm::cont::ArrayHandle<vtkm::Id> &nearestNeighborIds,
                       vtkm::cont::ArrayHandle<vtkm::FloatDefault> &distances,
                       DeviceAdapter device) const
  {
    vtkm::worklet::DispatcherMapField<PointLocatorWorklet, DeviceAdapter>().Invoke(
      points,
      nearestNeighborIds,
      distances,
      this->PrepareForExecution(device));
  }

  template<typename Type, typename Storage, typename DeviceAdapter>
  VTKM_CONT void FindNearestNeighbors(const vtkm::cont::ArrayHandleVirtualCoordinates &points,
                                      vtkm::cont::ArrayHandle<vtkm::Id> &nearestNeighborIds,
                                      vtkm::cont::ArrayHandle<vtkm::FloatDefault> &distances
                                      DeviceAdapter device) const
  {
    this->FindCells(vtkm::cont::ArrayHandleVirtualCoordiantes(points),
                    nearestNeighborIds,
                    distances,
                    device);
  }

  // The following method should be available as a general VTK-m utility.
  // Is it already?
  <typename DeviceAdapter>
  VTKM_CONT vtkm::cont::DeviceAdapterId GetDeviceId(DeviceAdapter device)
  {
    using DeviceInfo = vtkm::cont::DeviceAdapterTraits<DeviceAdapter>;
    vtkm::cont::DeviceAdapterId deviceId = DeviceInfo::GetId();
    if (deviceId < 0 || deviceId >= VTKM_MAX_DEVICE_ADAPTER_ID)
    {
      std::string msg = "Device '" + DeviceInfo::GetName() + "' has invalid ID of " +
      std::to_string(deviceId) + "(VTKM_MAX_DEVICE_ADAPTER_ID = " +
      std::to_string(VTKM_MAX_DEVICE_ADAPTER_ID) + ")";
      throw vtkm::cont::ErrorBadType(msg);
    }
    return deviceId;
  }

  template<typename DeviceAdapter>
  VTKM_CONT vtkm::exec::PointLocator PrepareForExecution(DeviceAdapter device)
  {
    return this->PrepareForExecution(GetDeviceId(device));
  }

  VTKM_CONT virtual vtkm::exec::PointLocator PrepareForExecution(vtkm::cont::DeviceAdapterId device) {} = 0;

private:
  vtkm::cont::CoordinateSystem coordinates;
  bool dirty;
}

} // namespace cont
} // namespace vtkm
