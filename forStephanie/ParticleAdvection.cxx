#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Timer.h>
#include <vtkm/io/reader/BOVDataSetReader.h>
#include <vtkm/io/writer/VTKDataSetWriter.h>

#include <vtkm/worklet/ParticleAdvection.h>
#include <vtkm/worklet/particleadvection/GridEvaluators.h>
#include <vtkm/worklet/particleadvection/Integrators.h>
#include <vtkm/worklet/particleadvection/ParticleAdvectionWorklets.h>
#include <vtkm/worklet/particleadvection/Particles.h>

#include "seeds.cxx"

#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

#ifdef __BUILDING_TBB_VERSION__
#include <tbb/task_scheduler_init.h>
#endif

template <typename T>
void ignore(T&&)
{}

int renderAndWriteDataSet(vtkm::cont::DataSet& dataset, std::string output)
{
  std::cout << "Writing the dataset" << std::endl;
  vtkm::io::writer::VTKDataSetWriter writer(output);
  writer.WriteDataSet(dataset, static_cast<vtkm::Id>(0));
  return 0;
}

void performAdvection(std::string inData,
                      int numSeeds,
                      int numSteps,
                      float length,
                      vtkm::cont::DataSet& outData)
{
  using DeviceAdapter = VTKM_DEFAULT_DEVICE_ADAPTER_TAG;
  using FieldType = vtkm::Float32;
  using FieldHandle = vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3>>;
  using FieldPortalConstType =
    typename FieldHandle::template ExecutionTypes<DeviceAdapter>::PortalConst;

  // Read the dataset and extract the proper velocity to work on.
  vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3>> fieldArray;
  vtkm::io::reader::BOVDataSetReader reader(inData);
  vtkm::cont::DataSet dataset = reader.ReadDataSet();
  dataset.GetField(0).GetData().CopyTo(fieldArray);

  // Generate random seeds for the advection example.
  std::vector<vtkm::Vec<FieldType, 3>> seeds;
  GenerateSeeds(seeds, dataset, numSeeds, inData);
  vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3>> seedsArray;
  seedsArray = vtkm::cont::make_ArrayHandle(seeds);

  using GridEvaluator =
    vtkm::worklet::particleadvection::UniformGridEvaluate<FieldPortalConstType,
                                                          FieldType,
                                                          DeviceAdapter>;
  using Integrator = vtkm::worklet::particleadvection::RK4Integrator<GridEvaluator, FieldType>;

  // Initialize the grid evaluator and the integrator for advection.
  GridEvaluator eval(dataset.GetCoordinateSystem(), dataset.GetCellSet(0), fieldArray);
  Integrator integrator(eval, length);

  // Launch the streamline worklet
  vtkm::worklet::Streamline streamline;
  vtkm::worklet::StreamlineResult<FieldType> res =
    streamline.Run(integrator, seedsArray, numSteps, DeviceAdapter());

  // Add required fields to the output dataset
  vtkm::cont::CoordinateSystem outputCoords("coordinates", res.positions);
  outData.AddCellSet(res.polyLines);
  outData.AddCoordinateSystem(outputCoords);
}

int main(int argc, char** argv)
{
  int numSeeds = 1000;
  int numSteps = 1000;
  float length = 0.5;
  int threads = 8;
  std::string datasetname;
  std::string outFile("output.vtk");
  if(argc < 2)
  {
    std::cout << "Incorrect number of parameters" << std::endl;
    std::cout << "Usage : ./executable \n"
              << " -data <dataset> | mandatory \n"
              << " -seeds <num seeds> | default 1000 \n"
              << " -steps <num steps> | default 1000 \n"
              << " -legth <step lenght> | default 0.5 \n"
              << " -t <num threads> | default XXX \n"
              << " -out <output name | default output.vtk \n";
    exit(EXIT_FAILURE);
  }
  for(int i = 1; i < argc; i+=2)
  {
    std::string param(argv[i]);
    if(param.compare("-data") == 0)
    {
      std::cout << argv[i+1] << std::endl;
      datasetname = std::string(argv[i+1]);
    }
    else if(param.compare("-seeds") == 0)
      numSeeds = atoi(argv[i+1]);
    else if(param.compare("-steps") == 0)
      numSteps = atoi(argv[i+1]);
    else if(param.compare("-length") == 0)
      length = static_cast<float>(atof(argv[i+1]));
    else if(param.compare("-t") == 0)
    {
      threads = atoi(argv[i+1]);
      #ifdef __BUILDING_TBB_VERSION__
        tbb::task_scheduler_init init(threads);
      #else
        ignore(threads);
      #endif
    }
    else if(param.compare("-out") == 0)
    {
      outFile = std::string(argv[i+1]);
    }
  }
  std::cout << "Executing with params : "
            << datasetname << " dataset, "
            << numSeeds << " seeds, "
            << numSteps << " steps, "
            << length << " length, "
            << threads << " threads."
            << std::endl;

  vtkm::cont::DataSet outData;

  // Begin timing
  vtkm::cont::Timer<VTKM_DEFAULT_DEVICE_ADAPTER_TAG> timer;

  performAdvection(datasetname, numSeeds, numSteps, length, outData);

  std::cout << "Time taken for advection : " << timer.GetElapsedTime() << std::endl;

  // Write the output dataset : This contains streamlines
  renderAndWriteDataSet(outData, outFile);
}
