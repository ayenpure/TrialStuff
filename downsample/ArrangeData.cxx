#include <iomanip>

#include <vtkm/cont/DataSet.h>
#include <vtkm/io/reader/VTKDataSetReader.h>

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    exit(1);
  }
  std::string datasetName(argv[1]);
  std::string fieldName(argv[2]);

  vtkm::io::reader::VTKDataSetReader reader(datasetName);
  vtkm::cont::DataSet dataset = reader.ReadDataSet();

  using Scalar = vtkm::FloatDefault;
  using Vector = vtkm::Vec<Scalar, 3>;

  vtkm::cont::ArrayHandle<Scalar> vectorField;
  dataset.GetField(fieldName).GetData().CopyTo(vectorField);

  auto portal = vectorField.GetPortalConstControl();
  vtkm::Id numValues = vectorField.GetNumberOfValues();
  std::setprecision(5);
  std::cout << std::fixed;
  for(vtkm::Id index = 0; index < numValues; index++)
  {
    auto value = portal.Get(index);
    //std::cout << value[0] << "f, " << value[1] << "f, " << value[2] << "f, ";
    std::cout << value << "f, ";
    if((index + 1)  % 9  == 0)
    {
      std::cout << std::endl;
    }
  }
}
