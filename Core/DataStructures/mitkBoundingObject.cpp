#include "mitkBoundingObject.h"
#include "vtkTransform.h"
#include "mitkVector.h"

mitk::BoundingObject::BoundingObject()
  : SurfaceData(), m_Positive(true)
{
  m_Geometry3D->Initialize();
}

mitk::BoundingObject::~BoundingObject() 
{
} 

void mitk::BoundingObject::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::BoundingObject::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return ! VerifyRequestedRegion();
}

bool mitk::BoundingObject::VerifyRequestedRegion()
{
  assert(m_Geometry3D.IsNotNull());
  return true;
}

void mitk::BoundingObject::SetRequestedRegion(itk::DataObject *data)
{
}

void mitk::BoundingObject::UpdateOutputInformation()
{  
  ScalarType bounds[6]={0,1,0,1,0,1};  //{xmin,x_max, ymin,y_max,zmin,z_max}
  /* bounding box around the unscaled bounding object */ 
  bounds[0] = - 1;
  bounds[1] = + 1;
  bounds[2] = - 1;
  bounds[3] = + 1;
  bounds[4] = - 1;
  bounds[5] = + 1; 
  m_Geometry3D->SetBounds(bounds);
}

mitk::ScalarType mitk::BoundingObject::GetVolume()
{
  return 0.0;
}
