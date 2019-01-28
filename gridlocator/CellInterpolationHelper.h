#ifndef cell_interpolation_helper
#define cell_interpolation_helper

#include <vtkm/Types.h>
#include <vtkm/CellShape.h>

#include <vtkm/exec/CellInterpolate.h>
#include <vtkm/cont/ExecutionObjectBase.h>

/*
 * Interface to define the helper classes that can return mesh data
 * on a cell by cell basis.
 */
namespace vtkm
{
namespace exec
{

class CellInterpolationHelper : public vtkm::VirtualObjectBase
{
public:
  VTKM_EXEC
  virtual vtkm::UInt8 GetCellShape(const vtkm::Id& cellId) const = 0;

  VTKM_EXEC
  virtual vtkm::UInt8 GetFieldIndices(const vtkm::Id& cellId, vtkm::Id fieldIndices[]) const = 0;
};

class StructuredCellInterpolationHelper : public vtkm::exec::CellInterpolationHelper
{
public:

  VTKM_CONT
  StructuredCellInterpolationHelper() = default;

  VTKM_CONT
  StructuredCellInterpolationHelper(vtkm::Id3 cellDims, vtkm::Id3 pointDims)
  : CellDims(cellDims)
  , PointDims(pointDims)
  {}

  VTKM_EXEC
  vtkm::UInt8 GetCellShape(const vtkm::Id& vtkmNotUsed(cellId)) const override
  {
    return vtkm::UInt8(vtkm::CELL_SHAPE_HEXAHEDRON);
  }

  VTKM_EXEC
  vtkm::UInt8 GetFieldIndices(const vtkm::Id& cellId, vtkm::Id fieldIndices[8]) const override
  {
    vtkm::Id3 logicalCellId;
    logicalCellId[0] = cellId % CellDims[0];
    logicalCellId[1] = (cellId / CellDims[0]) % CellDims[1];
    logicalCellId[2] = cellId / (CellDims[0] * CellDims[1]);
    fieldIndices[0] =
      (logicalCellId[2] * PointDims[1] + logicalCellId[1]) * PointDims[0] + logicalCellId[0];
    fieldIndices[1] = fieldIndices[0] + 1;
    fieldIndices[2] = fieldIndices[1] + PointDims[0];
    fieldIndices[3] = fieldIndices[2] - 1;
    fieldIndices[4] = fieldIndices[0] + PointDims[0] * PointDims[1];
    fieldIndices[5] = fieldIndices[4] + 1;
    fieldIndices[6] = fieldIndices[5] + PointDims[0];
    fieldIndices[7] = fieldIndices[6] - 1;
    return 8;
  }
private:
  vtkm::Id3 CellDims;
  vtkm::Id3 PointDims;
};

} // namespace exec

/*
 * Control side base object.
 */
namespace cont
{

class CellInterpolationHelper : public vtkm::cont::ExecutionObjectBase
{
public:
  using HandleType = vtkm::cont::VirtualObjectHandle<vtkm::exec::CellInterpolationHelper>;

  template <typename DeviceAdapter>
  VTKM_CONT const vtkm::exec::CellInterpolationHelper* PrepareForExecution(DeviceAdapter device) const
  {
    return PrepareForExecutionImpl(device).PrepareForExecution(device);
  }

protected:
  VTKM_CONT virtual const HandleType PrepareForExecutionImpl(
    const vtkm::cont::DeviceAdapterId device) const = 0;
};

class StructuredCellInterpolationHelper : public vtkm::cont::CellInterpolationHelper
{
public:
  using StructuredType = vtkm::cont::CellSetStructured<3>;

  VTKM_CONT
  StructuredCellInterpolationHelper() = default;

  VTKM_CONT
  StructuredCellInterpolationHelper(vtkm::cont::DynamicCellSet& cellSet)
  {
    if (!cellSet.IsSameType(StructuredType()))
      throw vtkm::cont::ErrorBadType("Cell set is not 3D structured type");
    CellDims =
      cellSet.Cast<StructuredType>().GetSchedulingRange(vtkm::TopologyElementTagCell());
    PointDims =
      cellSet.Cast<StructuredType>().GetSchedulingRange(vtkm::TopologyElementTagPoint());
  }

  struct StructuredCellFunctor
  {
    template <typename DeviceAdapter>
    VTKM_CONT bool operator()(DeviceAdapter,
                              const vtkm::cont::StructuredCellInterpolationHelper& contInterpolator,
                              HandleType& execInterpolator) const
    {
      using ExecutionType = vtkm::exec::StructuredCellInterpolationHelper;
      ExecutionType* execObject =
        new ExecutionType(contInterpolator.CellDims,
                          contInterpolator.PointDims);
      execInterpolator.Reset(execObject);
      return true;
    }
  };

  VTKM_CONT
  const HandleType PrepareForExecutionImpl(
    const vtkm::cont::DeviceAdapterId deviceId) const override
  {
    const bool success = vtkm::cont::TryExecuteOnDevice(
      deviceId, StructuredCellFunctor(), *this, this->ExecHandle);
    if (!success)
    {
      throwFailedRuntimeDeviceTransfer("StructuredCellInterpolationHelper", deviceId);
    }
    return this->ExecHandle;
  }
private:
  vtkm::Id3 CellDims;
  vtkm::Id3 PointDims;
  mutable HandleType ExecHandle;
};

} //namespace cont
} //namespace vtkm

#endif
