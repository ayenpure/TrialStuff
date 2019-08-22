#include "mpi.h"

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include <vtkm/Types.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Algorithm.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/Keys.h>

namespace detail
{

class IdentifyDomains : public vtkm::worklet::WorkletMapField
{
public:
  using ControlSignature = void(FieldIn, WholeArrayIn, FieldOut);

  using ExecutionSignature = void(_1, _2, _3);

  template <typename Point, typename BoundsPortal>
  VTKM_EXEC
  void operator()(const Point& point,
                  const BoundsPortal& bounds,
                  vtkm::Id& domain) const
  {
    for(vtkm::Id index = 0; index < bounds.GetNumberOfValues(); index++)
    {
      if((bounds.Get(index)).Contains(point))
      {
        domain = index;
        return;
      }
    }
    domain = -1;
  }
};

class GatherValuesToSend : public vtkm::worklet::WorkletMapField
{
public:
  using ControlSignature = void(FieldIn, WholeArrayIn, WholeArrayOut);

  using ExecutionSignature = void(WorkIndex, _1, _2, _3);

  template <typename TPortalRead, typename TPortalWrite>
  VTKM_EXEC
  void operator()(const vtkm::Id workIndex,
                  const vtkm::Id readIndex,
                  const TPortalRead& input,
                  TPortalWrite& output) const
  {
    auto vec = input.Get(readIndex);
    vtkm::Id writeIndex = workIndex * 3;
    output.Set(writeIndex++,vec[0]);
    output.Set(writeIndex++,vec[1]);
    output.Set(writeIndex, vec[2]);
  }
};

} //namespace detial

void GenerateSeeds(int rank,
                   const vtkm::Bounds& bounds,
                   std::vector<vtkm::Vec3f>& seeds)
{
  using Scalar = vtkm::FloatDefault;

  srand(rank);
  for(vtkm::Id index = 0; index < 1000; index++)
  {
    Scalar rx = static_cast<Scalar>(rand()) / static_cast<Scalar>(RAND_MAX);
    Scalar ry = static_cast<Scalar>(rand()) / static_cast<Scalar>(RAND_MAX);
    Scalar rz = static_cast<Scalar>(rand()) / static_cast<Scalar>(RAND_MAX);
    vtkm::Vec3f point;
    point[0] = static_cast<Scalar>(bounds.X.Min + rx * bounds.X.Length());
    point[1] = static_cast<Scalar>(bounds.Y.Min + ry * bounds.Y.Length());
    point[2] = static_cast<Scalar>(bounds.Z.Min + rz * bounds.Z.Length());
    seeds.push_back(point);
  }
}

void Gather(vtkm::cont::ArrayHandle<vtkm::FloatDefault>& toSend,
            vtkm::Id offset,
            vtkm::IdComponent counts,
            const vtkm::cont::ArrayHandle<vtkm::Vec3f>& seedsHandle,
            const vtkm::cont::ArrayHandle<vtkm::Id>& gatherMap)
{
  vtkm::cont::ArrayHandle<vtkm::Id> valuesToGather;
  using Algorithm = vtkm::cont::Algorithm;
  Algorithm::CopySubRange(gatherMap, offset, counts, valuesToGather);

  toSend.Allocate(counts * 3);
  vtkm::worklet::DispatcherMapField<detail::GatherValuesToSend> dispatcher;
  dispatcher.Invoke(valuesToGather, seedsHandle, toSend);
  toSend.SyncControlArray();
}

void SendSeedsToProcesses(const int rank,
                          const std::map<int, vtkm::cont::ArrayHandle<vtkm::FloatDefault>>& sendMap,
                          const std::vector<vtkm::Bounds>& bounds)
{
  using Scalar = vtkm::FloatDefault;

  // Do a multi pass
  // 1. Allocate the buffers using the amount of seeds to send
  // 2. Perform the send operation to all ranks
  // 3. Receive data from all ranks (should be done in another method)
  int buffer_size = 0;
  for (auto const& rankToSeeds : sendMap)
  {
    int sendRank = rankToSeeds.first;
    if(sendRank == rank)
      continue;
    vtkm::cont::ArrayHandle<Scalar> seeds =  rankToSeeds.second;
    int numValues = seeds.GetNumberOfValues();
    buffer_size += numValues;
    std::cout << "[" << rank << "] Exchanging with rank :: " << sendRank << " | ";
    std::cout << "Number of seeds to exchange :: " << numValues
              << "(" << numValues / 3 << ")" << std::endl;
    // Make a strategy to send these seeds async, using non-blocking communication.
    // Async will let us send all seeds at once without blocking.
    // This will hopefully be fast,
    // Compare
    // MPI_Bsend vs MPI_Isend for sending this same data.
    // If 100 seeds are too few to get good times, gather 1000/10000 seeds per node
  }
  buffer_size += MPI_BSEND_OVERHEAD;
  void* buffer = malloc(buffer_size*sizeof(Scalar));
  int detach_size;
  MPI_Buffer_detach(&buffer, &detach_size);
  MPI_Buffer_attach(buffer, buffer_size);
  for (auto const& rankToSeeds : sendMap)
  {
    int sendRank = rankToSeeds.first;
    if(sendRank == rank)
      continue;
    vtkm::cont::ArrayHandle<Scalar> seeds =  rankToSeeds.second;
    int numValues = seeds.GetNumberOfValues();
    void* seedPointer = seeds.GetStorage().GetBasePointer();
    MPI_Bsend(seedPointer, numValues, MPI_FLOAT, sendRank, 0, MPI_COMM_WORLD);
    seeds.ReleaseResources();
  }
}

bool Reduce(bool* receives)
{
  bool toReturn = true;
  for(int i = 0; i < 4; i++)
  {
    toReturn = toReturn && receives[i];
  }
  return toReturn;
}

void ReceiveSeedsFromProcesses(const int rank,
                               const std::vector<vtkm::Bounds>& bounds)
{
  // Probe for messages from a certain rank
  // Receive the seeds from that rank
  // Initially just print the probe results
  bool receives[4];
  receives[rank] = true;
  bool allDone = false;
  while(!allDone)
  {
    MPI_Status p_stat, r_stat;
    int probe_err = MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &p_stat);
    if(receives[p_stat.MPI_SOURCE] == true)
      continue;
    int receivable;
    MPI_Get_count(&p_stat, MPI_FLOAT, &receivable);
    std::cout << "["<< rank <<"]" << " To receive :: " << receivable
              << "(" << receivable / 3 << ")"
              << " from " << p_stat.MPI_SOURCE << std::endl;
    // Make receive buffer

    using Scalar = vtkm::FloatDefault;
    Scalar* buffer = (Scalar*)malloc(receivable*sizeof(Scalar));
    // Do MPI_Recv
    MPI_Recv(buffer, receivable, MPI_FLOAT, p_stat.MPI_SOURCE, 0, MPI_COMM_WORLD, &r_stat);
    receives[p_stat.MPI_SOURCE] = true;
    allDone = Reduce(receives);
  }
}

void ProcessSeeds(const int rank,
                  const std::vector<vtkm::Vec3f>& seeds,
                  const std::vector<vtkm::Bounds>& bounds)
{
  using Scalar = vtkm::FloatDefault;

  vtkm::cont::ArrayHandle<vtkm::Vec3f> seedsHandle = vtkm::cont::make_ArrayHandle(seeds);
  vtkm::cont::ArrayHandle<vtkm::Bounds> boundsHandle = vtkm::cont::make_ArrayHandle(bounds);
  vtkm::cont::ArrayHandle<vtkm::Id> domainForSeeds;

  vtkm::worklet::DispatcherMapField<detail::IdentifyDomains> dispatcher;
  dispatcher.Invoke(seedsHandle, boundsHandle, domainForSeeds);

  // Identify number of unique domains
  //  -- This can be only achieved by sorting
  // Scan the domainForSeeds array to group points into Vectors
  // How much faster is this compared to the serial approach that Dave has?
  vtkm::worklet::Keys<vtkm::Id> domainKeys;
  domainKeys.BuildArrays(domainForSeeds, vtkm::worklet::KeysSortType::Stable);

  vtkm::cont::ArrayHandle<vtkm::Id> domainToSeedIndices = domainKeys.GetSortedValuesMap();
  vtkm::cont::ArrayHandle<vtkm::Id> offsets = domainKeys.GetOffsets();
  vtkm::cont::ArrayHandle<vtkm::IdComponent> counts = domainKeys.GetCounts();

  vtkm::cont::ArrayHandle<Scalar> toSend0;
  vtkm::cont::ArrayHandle<Scalar> toSend1;
  vtkm::cont::ArrayHandle<Scalar> toSend2;
  vtkm::cont::ArrayHandle<Scalar> toSend3;

  std::map<int, vtkm::cont::ArrayHandle<Scalar>> sendMap;
  auto offsetPortal = offsets.GetPortalConstControl();
  auto countsPortal = counts.GetPortalConstControl();
  if(rank == 0)
  {
    // Send to Rank 1, 2, 3;
    Gather(toSend1, offsetPortal.Get(1), countsPortal.Get(1), seedsHandle, domainToSeedIndices);
    Gather(toSend2, offsetPortal.Get(2), countsPortal.Get(2), seedsHandle, domainToSeedIndices);
    Gather(toSend3, offsetPortal.Get(3), countsPortal.Get(3), seedsHandle, domainToSeedIndices);
    sendMap[1] = toSend1;
    sendMap[2] = toSend2;
    sendMap[3] = toSend3;
  }
  if(rank == 1)
  {
    // Send to Rank 0, 2, 3;
    Gather(toSend0, offsetPortal.Get(0), countsPortal.Get(0), seedsHandle, domainToSeedIndices);
    Gather(toSend2, offsetPortal.Get(2), countsPortal.Get(2), seedsHandle, domainToSeedIndices);
    Gather(toSend3, offsetPortal.Get(3), countsPortal.Get(3), seedsHandle, domainToSeedIndices);
    sendMap[0] = toSend0;
    sendMap[2] = toSend2;
    sendMap[3] = toSend3;
  }
  if(rank == 2)
  {
    // Send to Rank 0, 1, 3;
    Gather(toSend0, offsetPortal.Get(0), countsPortal.Get(0), seedsHandle, domainToSeedIndices);
    Gather(toSend1, offsetPortal.Get(1), countsPortal.Get(1), seedsHandle, domainToSeedIndices);
    Gather(toSend3, offsetPortal.Get(3), countsPortal.Get(3), seedsHandle, domainToSeedIndices);
    sendMap[0] = toSend0;
    sendMap[1] = toSend1;
    sendMap[3] = toSend3;
  }
  if(rank == 3)
  {
    // Send to Rank 0, 1, 2;
    Gather(toSend0, offsetPortal.Get(0), countsPortal.Get(0), seedsHandle, domainToSeedIndices);
    Gather(toSend1, offsetPortal.Get(1), countsPortal.Get(1), seedsHandle, domainToSeedIndices);
    Gather(toSend2, offsetPortal.Get(2), countsPortal.Get(2), seedsHandle, domainToSeedIndices);
    sendMap[0] = toSend0;
    sendMap[1] = toSend1;
    sendMap[2] = toSend2;
  }
  SendSeedsToProcesses(rank, sendMap, bounds);

  ReceiveSeedsFromProcesses(rank, bounds);
}

int main(int argc, char** argv)
{
  int rank, processes;
  char hostName[MPI_MAX_PROCESSOR_NAME];
  int nameLength;

  MPI_Init(&argc, &argv);
  MPI_Get_processor_name(hostName, &nameLength);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);

  std::cout << "Process ("<< hostName<< ") : "
            << rank << "(" << processes << ")" << std::endl;

  vtkm::Bounds domainBounds(0., 10., 0., 10., 0., 10.);
  vtkm::Bounds bounds0(0., 5., 0., 5., 0., 10.);
  vtkm::Bounds bounds1(5., 10., 0., 5., 0., 10.);
  vtkm::Bounds bounds2(0., 5., 5., 10., 0., 10.);
  vtkm::Bounds bounds3(0., 10., 5., 10., 0., 10.);

  std::vector<vtkm::Bounds> rankToBounds;
  rankToBounds.push_back(bounds0);
  rankToBounds.push_back(bounds1);
  rankToBounds.push_back(bounds2);
  rankToBounds.push_back(bounds3);

  std::vector<vtkm::Vec3f> seeds;
  GenerateSeeds(rank, domainBounds, seeds);

  ProcessSeeds(rank, seeds, rankToBounds);

  MPI_Finalize();
}
