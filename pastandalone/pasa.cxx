#include <cstdlib>
#include <fstream>
#include <vector>

#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Timer.h>
#include <vtkm/io/reader/BOVDataSetReader.h>
#include <vtkm/io/reader/VTKDataSetReader.h>
#include <vtkm/worklet/ParticleAdvection.h>
#include <vtkm/worklet/particleadvection/GridEvaluators.h>
#include <vtkm/worklet/particleadvection/Integrators.h>
#include <vtkm/worklet/particleadvection/ParticleAdvectionWorklets.h>
#include <vtkm/worklet/particleadvection/Particles.h>

#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

#ifdef __BUILDING_TBB_VERSION__
#include <tbb/task_scheduler_init.h>
#endif

int main(int argc, char **argv) {

  if (argc < 3) {
    std::cout << "Wrong number of arguments" << std::endl;
    std::cout << "Usage : pasa <datafile> <flag for streamlines>" << std::endl;
    exit(EXIT_FAILURE);
  }

  using DeviceAdapter = VTKM_DEFAULT_DEVICE_ADAPTER_TAG;
  using FieldType = vtkm::Float32;
  using FieldHandle = vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3>>;
  using FieldPortalConstType =
      typename FieldHandle::template ExecutionTypes<DeviceAdapter>::PortalConst;

  std::string datafile(argv[1]);
  std::string seedsfile(argv[2]);

  int flag = atoi(argv[3]);
  vtkm::Id numSteps = 10000;
  FieldType stepSize = 0.0025;

#ifdef __BUILDING_TBB_VERSION__
  std::cout << "Executing for TBB" << std::endl;
  int numThreads = atoi(argv[4]);
  int nT = tbb::task_scheduler_init::default_num_threads();
  if (numThreads != -1)
    nT = (int)numThreads;
  // make sure the task_scheduler_init object is in scope when running sth w/
  // TBB
  tbb::task_scheduler_init init(nT);
#endif

  // Could have either vtk or bov
  vtkm::io::reader::VTKDataSetReader reader(datafile.c_str());
  vtkm::cont::DataSet dataset = reader.ReadDataSet();

  // Change according to file format
  using RGEvalType = vtkm::worklet::particleadvection::RectilinearGridEvaluate<
      FieldPortalConstType, FieldType, DeviceAdapter>;
  using RK4RGType =
      vtkm::worklet::particleadvection::RK4Integrator<RGEvalType, FieldType>;

  vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3>> fieldArray;
  dataset.GetField(0).GetData().CopyTo(fieldArray);

  RGEvalType eval(dataset.GetCoordinateSystem(), dataset.GetCellSet(0),
                  fieldArray);
  RK4RGType rk4(eval, stepSize);

  std::vector<vtkm::Vec<FieldType, 3>> seeds;
  double x,y,z;
  std::ifstream infile;
  infile.open(seedsfile);
  while(infile)
  {
    infile >> x >> y >> z;
    seeds.push_back(vtkm::Vec<FieldType,3>(x,y,z));
  }
  infile.close();
  std::cout << "Number of seeds : " << seeds.size()  << std::endl;
  vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3>> seedArray;
  seedArray = vtkm::cont::make_ArrayHandle(seeds);

  vtkm::cont::Timer<VTKM_DEFAULT_DEVICE_ADAPTER_TAG> advectTimer;

  if (flag == 0) {
    std::cout << "Executing Particle Advection" << std::endl;
    vtkm::worklet::ParticleAdvection particleAdvection;
    particleAdvection.Run(rk4, seedArray, numSteps, DeviceAdapter());
  } else {
    std::cout << "Executing Streamlines" << std::endl;
    vtkm::worklet::Streamline streamline;
    streamline.Run(rk4, seedArray, numSteps, DeviceAdapter());
  }
  std::cout << "Time taken for advection : " << advectTimer.GetElapsedTime()
            << std::endl;
}
