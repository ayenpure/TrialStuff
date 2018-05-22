template<typename DerievedLocator>
class PointLocator
{
public:
  PointLocator();

  vtkm::cont::DynamicCoordinateSystem GetCoords() const;

  // Set the coodrinate system the current point locator is supposed
  // to operate over.
  void SetCoords(const vtkm::cont::DynamicCoordinateSystem& coords);

  //Builds the search structure and creates the Execution Obejct
  template<typename DeviceAdapter>
  virtual void Build(DeviceAdapter) = 0;

  template<typename PointComponentType,
           typename PointStorageType,
           typename DeviceAdapter>
  virtual void FindNearestNeighbor(
    const vtkm::cont::ArrayHandle<vtkm::Vec<PointComponentType, 3>, PointStorageType>& coords,
    const vtkm::cont::ArrayHandle<vtkm::Vec<PointComponentType, 3>, PointStorageType>& queryPoints,
    vtkm::cont::ArrayHandle<vtkm::Id>& nearestNeighborIds,
    vtkm::cont::ArrayHandle<PointComponentType> distances,
    DeviceAdapter) const = 0;

protected:
  //The basic needed attributes on the point locator.
  vtkm::cont::CoordinateSystem coordinates;
  vtkm::cont::ExecutionObjectFactoryBase executionObject;
};
