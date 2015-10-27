/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkImageVtkXmlIO.h"

#include "mitkImage.h"
#include "mitkIOMimeTypes.h"
#include "mitkImageVtkReadAccessor.h"

#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageData.h>
#include <vtkErrorCode.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <itkEuler3DTransform.h>

namespace mitk {

class VtkXMLImageDataReader : public ::vtkXMLImageDataReader
{
public:
  static VtkXMLImageDataReader *New() { return new VtkXMLImageDataReader(); }
  vtkTypeMacro(VtkXMLImageDataReader,vtkXMLImageDataReader)

  void SetStream(std::istream* is) { this->Stream = is; }
  std::istream* GetStream() const { return this->Stream; }
};

class VtkXMLImageDataWriter : public ::vtkXMLImageDataWriter
{
public:
  static VtkXMLImageDataWriter *New() { return new VtkXMLImageDataWriter(); }
  vtkTypeMacro(VtkXMLImageDataWriter,vtkXMLImageDataWriter)

  void SetStream(std::ostream* os) { this->Stream = os; }
  std::ostream* GetStream() const { return this->Stream; }
};

#define DEFINE_STRING(name, value) \
    static const std::string& name() { static const std::string r = value; return r; };

DEFINE_STRING(OUTPUT_TYPE, "org.mitk.io.Output type")
DEFINE_STRING(OUTPUT_TYPE_ENUM, OUTPUT_TYPE() + ".enum")
DEFINE_STRING(OUTPUT_TYPE_VTK, "VTK (vti only)")
DEFINE_STRING(OUTPUT_TYPE_PARAVIEW, "Paraview (vti + py)")

ImageVtkXmlIO::ImageVtkXmlIO()
  : AbstractFileIO(Image::GetStaticNameOfClass(), IOMimeTypes::VTK_IMAGE_MIMETYPE(), "VTK XML Image")
{

  Options defaultOptions;
  defaultOptions[OUTPUT_TYPE()] = OUTPUT_TYPE_VTK();
  defaultOptions[OUTPUT_TYPE_ENUM()] = std::vector<std::string>{OUTPUT_TYPE_VTK(), OUTPUT_TYPE_PARAVIEW()};
  this->SetDefaultWriterOptions(defaultOptions);

  this->RegisterService();
}

std::vector<BaseData::Pointer> ImageVtkXmlIO::Read()
{
  vtkSmartPointer<VtkXMLImageDataReader> reader = vtkSmartPointer<VtkXMLImageDataReader>::New();
  if (this->GetInputStream())
  {
    reader->SetStream(this->GetInputStream());
  }
  else
  {
    reader->SetFileName(this->GetInputLocation().c_str());
  }
  reader->Update();

  if (reader->GetOutput() != NULL)
  {
    auto pointData = reader->GetOutput()->GetPointData();

    if (pointData->GetScalars() == nullptr) 
    {
        for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) 
        {
            if (pointData->GetArray(i)->GetNumberOfComponents() == 1) 
            {
                pointData->SetActiveAttribute(i, vtkDataSetAttributes::SCALARS);
                break;
            }
        }
    }

    if (pointData->GetScalars() == nullptr) 
    {
        mitkThrow() << "mitkImageVtkXmlIO error: could not find scalars in the image.";
    }

    mitk::Image::Pointer output = mitk::Image::New();
    output->Initialize(reader->GetOutput());
    output->SetVolume(pointData->GetScalars()->GetVoidPointer(0));
    std::vector<BaseData::Pointer> result;
    result.push_back(output.GetPointer());
    return result;
  }
  else
  {
    mitkThrow() << "vtkXMLImageDataReader error: " << vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode());
  }
}

IFileIO::ConfidenceLevel ImageVtkXmlIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported) return Unsupported;
  if (this->GetInputStream() == NULL)
  {
    // check if the xml vtk reader can handle the file
    vtkSmartPointer<VtkXMLImageDataReader> xmlReader = vtkSmartPointer<VtkXMLImageDataReader>::New();
    if (xmlReader->CanReadFile(this->GetInputLocation().c_str()) != 0)
    {
      return Supported;
    }
    return Unsupported;
  }
  // in case of an input stream, VTK does not seem to have methods for
  // validating it
  return Supported;
}

void ImageVtkXmlIO::Write()
{
  ValidateOutputLocation();

  const Image* input = dynamic_cast<const Image*>(this->GetInput());

  vtkSmartPointer<VtkXMLImageDataWriter> writer = vtkSmartPointer<VtkXMLImageDataWriter>::New();
  if (this->GetOutputStream())
  {
    writer->SetStream(this->GetOutputStream());
  }
  else
  {
    writer->SetFileName(this->GetOutputLocation().c_str());
  }

  auto outputType = GetWriterOptions().find(OUTPUT_TYPE())->second.ToString();

  auto outputImage = mitk::Image::ConstPointer{};
  if (outputType == OUTPUT_TYPE_VTK()) {
      outputImage = input;
  }
  else if (outputType == OUTPUT_TYPE_PARAVIEW()) {
      auto imageCopy = input->Clone();
      imageCopy->GetGeometry()->SetIdentity();

      auto eulerTransform = itk::Euler3DTransform<double>::New();

      auto m = input->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();

      m.set_column(0, m.get_column(0).normalize());
      m.set_column(1, m.get_column(1).normalize());
      m.set_column(2, m.get_column(2).normalize());

      eulerTransform->SetComputeZYX(true);
      eulerTransform->SetMatrix(itk::Matrix<double>{m}, 1e-4);

      std::ofstream pyFile{ this->GetOutputLocation() + ".py" };

      pyFile << "from paraview.simple import *\n";
      pyFile << "img = XMLImageDataReader(FileName=r'" << GetOutputLocation() << "')\n";
      pyFile << "renderView = CreateView('RenderView')\n";
      pyFile << "imgDisplay = Show(img, renderView)\n";

      pyFile << "imgDisplay.Position = [" << input->GetGeometry()->GetOrigin()[0] << ", " << input->GetGeometry()->GetOrigin()[1] << ", " << input->GetGeometry()->GetOrigin()[2] << "]\n";
      pyFile << "imgDisplay.Scale = [" << input->GetGeometry()->GetSpacing()[0] << ", " << input->GetGeometry()->GetSpacing()[1] << ", " << input->GetGeometry()->GetSpacing()[2] << "]\n";
      pyFile << "imgDisplay.Orientation = ["
          << eulerTransform->GetAngleX() * 180 / vnl_math::pi << ", "
          << eulerTransform->GetAngleY() * 180 / vnl_math::pi << ", "
          << eulerTransform->GetAngleZ() * 180 / vnl_math::pi << "]\n";

      outputImage = imageCopy;
  }
  else {
      mitkThrow() << "Unknown output type";
  }

  ImageVtkReadAccessor vtkReadAccessor(Image::ConstPointer(input), NULL, outputImage->GetVtkImageData());
  writer->SetInputData(const_cast<vtkImageData*>(vtkReadAccessor.GetVtkImageData()));

  if (writer->Write() == 0 || writer->GetErrorCode() != 0 )
  {
    mitkThrow() << "vtkXMLImageDataWriter error: " << vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode());
  }
}

IFileIO::ConfidenceLevel ImageVtkXmlIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported) return Unsupported;
  const Image* input = static_cast<const Image*>(this->GetInput());
  if (input->GetDimension() == 3) return Supported;
  else if (input->GetDimension() < 3) return PartiallySupported;
  return Unsupported;
}

ImageVtkXmlIO* ImageVtkXmlIO::IOClone() const
{
  return new ImageVtkXmlIO(*this);
}

}
