#include <vector>

#include <vtkm/Types.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleIndex.h>
#include <vtkm/cont/ArrayHandleZip.h>
#include <vtkm/cont/DeviceAdapterAlgorithm.h>
#include <vtkm/cont/serial/DeviceAdapterSerial.h>


namespace detail
{

using T = vtkm::Pair<vtkm::Id, vtkm::Id>;

struct compare1
{
  VTKM_EXEC_CONT
  bool operator()(const T& x, const T& y) const
  { return x.first < y.first;}
};

struct compare2
{
  VTKM_EXEC_CONT
  bool operator()(const T& x, const T& y) const
  { return x.second < y.second;}
};

}

int main()
{

  // Trying to replicate the case for the uniform grid approach.

  // i/p : 5 5 2 2 3 5 1 2 4 1
  // idx : 0 1 2 3 4 5 6 7 8 9
  // o/p : 0 0 2 2 4 0 6 2 8 6 <- Expected

  // SortByKey + ScanInclusiveByKey o/p
  // i/p : 1 1 2 2 2 3 4 5 5 5
  // o/p : 7 7 2 2 2 4 8 0 0 0 <- Nope!!

  using DeviceAdapter = vtkm::cont::DeviceAdapterTagSerial;
  using DeviceAlgorithm = typename vtkm::cont::DeviceAdapterAlgorithm<DeviceAdapter>;

  std::vector<vtkm::Id> cellIdVec{5, 5, 2, 2, 3, 5, 1, 2, 4, 1};
  using IdHandle = vtkm::cont::ArrayHandle<vtkm::Id>;
  IdHandle cellIds = vtkm::cont::make_ArrayHandle(cellIdVec);
  vtkm::cont::ArrayHandleIndex indices(cellIds.GetNumberOfValues());
  IdHandle sortAid;
  IdHandle pointIds, mergedIds;
  DeviceAlgorithm::Copy(indices, sortAid);
  DeviceAlgorithm::Copy(indices, pointIds);

  vtkm::cont::ArrayHandleZip<IdHandle, IdHandle> zipped(cellIds, sortAid);
  DeviceAlgorithm::SortByKey(zipped, pointIds, detail::compare1()/*comparator on 1st element*/);
  DeviceAlgorithm::ScanInclusiveByKey(cellIds, pointIds, mergedIds, vtkm::Minimum());
  DeviceAlgorithm::SortByKey(zipped, mergedIds, detail::compare2()/*comparator on 2nd element*/);

  // By this time mergedIds must contain the final output, Hopefully!!
  auto portal = mergedIds.GetPortalConstControl();
  for(int i = 0; i < mergedIds.GetNumberOfValues(); i++)
  { std::cout << portal.Get(i) << " "; }
  std::cout << std::endl;
  return 0;
}
