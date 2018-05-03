#include <cstdlib>

// Headers to deal with ArrayHandles
#include <vtkm/Types.h>
#include <vtkm/cont/ArrayHandle.h>

// Headers to deal with DataSets
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetFieldAdd.h>

#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

/*
 * The file contains demo code to create an ArrayHandle from an array,
 * and to add an array directly into a DataSet as a Field.
 */
int main(int argc, char **argv) {

  if(argc < 2)
  {
    std::cout << "Wrong number of arguments passed." << std::endl;
    exit(EXIT_FAILURE);
  }
  vtkm::Id numElements = atoi(argv[1]);

  std::cout << "Making array handle off an array." << std::endl;
  vtkm::FloatDefault* array = new vtkm::FloatDefault[numElements];
  for(int i = 0; i < numElements; i++)
  {
    array[i] = static_cast<vtkm::FloatDefault>(i);
  }


  // Code to make an ArrayHandle form an array.
  vtkm::cont::ArrayHandle<vtkm::FloatDefault> arrayHandle;
  arrayHandle = vtkm::cont::make_ArrayHandle(array, numElements, vtkm::CopyFlag::On);

  // Get the portal to the new ArrayHandle and print the contents.
  auto portal = arrayHandle.GetPortalConstControl();
  std::cout << "Number of Elements : " << portal.GetNumberOfValues() << std::endl;
  for(int i = 0; i < portal.GetNumberOfValues(); i++)
  {
    std::cout << portal.Get(i) << "\t";
  }
  std::cout << std::endl;


  // Code to add an array to a Dataset directly.
  const vtkm::Id nVerts = numElements;

  // You can ignore the detail that I am not actually creating a
  // dataset as you already have access to the dataset.
  vtkm::cont::DataSet dataset;

  // Here pointvar is the variable name for the DataSet.
  vtkm::cont::DataSetFieldAdd datasetFieldAdd;
  datasetFieldAdd.AddPointField(dataset, "pointvar", array, nVerts);

  std::cout << "Exiting successfully"  << std::endl;
  return 0;
}
