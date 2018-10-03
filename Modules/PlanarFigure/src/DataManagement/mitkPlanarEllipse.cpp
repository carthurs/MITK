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

#include "mitkPlanarEllipse.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

mitk::PlanarEllipse::PlanarEllipse()
    : FEATURE_ID_RADIUS1(this->AddFeature("Radius 1", "mm")),
      FEATURE_ID_RADIUS2(this->AddFeature("Radius 2", "mm")),
      FEATURE_ID_CIRCUMFERENCE(this->AddFeature("Circumference", "mm")),
      FEATURE_ID_AREA(this->AddFeature("Area", "mm2")), 
      m_MinRadius(0),
      m_MaxRadius(100),
      m_MinMaxRadiusContraintsActive(false),
      m_TreatAsCircle(true)
{
    // Ellipse has three control points
    this->SetNumberOfPolyLines( 1 );
    this->SetProperty( "closed", mitk::BoolProperty::New(true) );
}

bool mitk::PlanarEllipse::SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist)
{
  if (index == 0) // moving center point and control points accordingly
  {
    const Point2D &centerPoint = GetControlPoint(0);
    Point2D boundaryPoint1 = GetControlPoint(1);
    Point2D boundaryPoint2 = GetControlPoint(2);
    const vnl_vector<ScalarType> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

    boundaryPoint1[0] += vec[0];
    boundaryPoint1[1] += vec[1];
    boundaryPoint2[0] += vec[0];
    boundaryPoint2[1] += vec[1];
    PlanarFigure::SetControlPoint(0, point, createIfDoesNotExist);
    PlanarFigure::SetControlPoint(1, boundaryPoint1, createIfDoesNotExist);
    PlanarFigure::SetControlPoint(2, boundaryPoint2, createIfDoesNotExist);
    return true;
  }
  else if (index < 3)
  {
    PlanarFigure::SetControlPoint(index, point, createIfDoesNotExist);
    int otherIndex = index + 1;
    if (otherIndex > 2)
      otherIndex = 1;

    const Point2D &centerPoint = GetControlPoint(0);
    Point2D otherPoint = GetControlPoint(otherIndex);

    const Vector2D vec1 = point - centerPoint;
    Vector2D vec2;

		if (index == 1 && !m_FigureFinalized)
    {
      const float x = vec1[0];
      vec2[0] = vec1[1];
      vec2[1] = x;

      if (index == 1)
        vec2[0] *= -1;
      else
        vec2[1] *= -1;

      otherPoint = centerPoint + vec2;
      PlanarFigure::SetControlPoint(otherIndex, otherPoint, createIfDoesNotExist);

      }
    else if (vec1.GetNorm() > 0)
    {
      const float r = centerPoint.EuclideanDistanceTo(otherPoint);
      const float x = vec1[0];
      vec2[0] = vec1[1];
      vec2[1] = x;

      if (index == 1)
        vec2[0] *= -1;
      else
        vec2[1] *= -1;

      vec2.Normalize();
      vec2 *= r;

      if (vec2.GetNorm() > 0)
      {
        otherPoint = centerPoint + vec2;
        PlanarFigure::SetControlPoint(otherIndex, otherPoint, createIfDoesNotExist);
      }

      m_TreatAsCircle = false;
    }
    return true;
  }
  else if (index == 3)
  {
    const Point2D centerPoint = GetControlPoint(0);
    Vector2D vec3 = point - centerPoint;
    vec3.Normalize();
    const double r1 = centerPoint.EuclideanDistanceTo(GetControlPoint(1));
    const double r2 = centerPoint.EuclideanDistanceTo(GetControlPoint(2));
    const Point2D newPoint = centerPoint + vec3 * std::max(r1, r2);
    PlanarFigure::SetControlPoint(index, newPoint, createIfDoesNotExist);
    m_TreatAsCircle = false;
    return true;
  }
  return false;
}


mitk::Point2D mitk::PlanarEllipse::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
	if (!m_FigureFinalized) {
		return point;
	}

  Point2D indexPoint;
  this->GetPlaneGeometry()->WorldToIndex(point, indexPoint);

  BoundingBox::BoundsArrayType bounds = this->GetPlaneGeometry()->GetBounds();
  if (indexPoint[0] < bounds[0])
  {
    indexPoint[0] = bounds[0];
  }
  if (indexPoint[0] > bounds[1])
  {
    indexPoint[0] = bounds[1];
  }
  if (indexPoint[1] < bounds[2])
  {
    indexPoint[1] = bounds[2];
  }
  if (indexPoint[1] > bounds[3])
  {
    indexPoint[1] = bounds[3];
  }

  Point2D constrainedPoint;
  this->GetPlaneGeometry()->IndexToWorld(indexPoint, constrainedPoint);

  if (m_MinMaxRadiusContraintsActive)
  {
    if (index != 0)
    {
      const Point2D &centerPoint = this->GetControlPoint(0);
      const double euclideanDinstanceFromCenterToPoint1 = centerPoint.EuclideanDistanceTo(point);

      Vector2D vectorProjectedPoint = point - centerPoint;
      vectorProjectedPoint.Normalize();

      if (euclideanDinstanceFromCenterToPoint1 > m_MaxRadius)
      {
        vectorProjectedPoint *= m_MaxRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
      else if (euclideanDinstanceFromCenterToPoint1 < m_MinRadius)
      {
        vectorProjectedPoint *= m_MinRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
    }
  }

  return constrainedPoint;
	}

	void mitk::PlanarEllipse::GeneratePolyLine()
	{
  // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
  this->ClearPolyLines();

  const Point2D &centerPoint = GetControlPoint(0);
  const Point2D &boundaryPoint1 = GetControlPoint(1);
  const Point2D &boundaryPoint2 = GetControlPoint(2);

  Vector2D dir = boundaryPoint1 - centerPoint;
  dir.Normalize();
  vnl_matrix_fixed<float, 2, 2> rot;

  // differentiate between clockwise and counterclockwise rotation
  int start = 0;
  int end = 64;
  if (dir[1] < 0)
  {
    dir[0] = -dir[0];
    start = -32;
    end = 32;
  }
  // construct rotation matrix to align ellipse with control point vector
  rot[0][0] = dir[0];
  rot[1][1] = rot[0][0];
  rot[1][0] = sin(acos(rot[0][0]));
  rot[0][1] = -rot[1][0];

  const double radius1 = centerPoint.EuclideanDistanceTo(boundaryPoint1);
  const double radius2 = centerPoint.EuclideanDistanceTo(boundaryPoint2);

  // Generate poly-line with 64 segments
  for (int t = start; t < end; ++t)
  {
    const double alpha = (double)t * vnl_math::pi / 32.0;

    // construct the new polyline point ...
    vnl_vector_fixed<float, 2> vec;
    vec[0] = radius1 * cos(alpha);
    vec[1] = radius2 * sin(alpha);
    vec = rot * vec;

    Point2D polyLinePoint;
    polyLinePoint[0] = centerPoint[0] + vec[0];
    polyLinePoint[1] = centerPoint[1] + vec[1];

    // ... and append it to the PolyLine.
    // No extending supported here, so we can set the index of the PolyLineElement to '0'
    this->AppendPointToPolyLine(0, polyLinePoint);
  }

		//     AppendPointToPolyLine( 1, PolyLineElement( centerPoint, 0 ) );
		//     AppendPointToPolyLine( 1, PolyLineElement( GetControlPoint( 3 ), 0 ) );
	}

	void mitk::PlanarEllipse::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
	{
  // A circle does not require a helper object
	}

	void mitk::PlanarEllipse::EvaluateFeaturesInternal()
	{
		const Point2D centerPoint = this->GetControlPoint(0);
		// Calculate circle radius and area
		const Point3D &p0 = this->GetWorldControlPoint(0);
		const Point3D &p1 = this->GetWorldControlPoint(1);
		const Point3D &p2 = this->GetWorldControlPoint(2);

		double radius1 = p0.EuclideanDistanceTo(p1);
		double radius2 = p0.EuclideanDistanceTo(p2);
		double h = (radius1 - radius2) * (radius1 - radius2) / ((radius1 + radius2) * (radius1 + radius2));
		double circumference = vnl_math::pi * (radius1 + radius2) * (1 + 3 * h / (10 + sqrt(4 - 3 * h)));
		double area = vnl_math::pi * radius1 * radius2;

  this->SetQuantity(FEATURE_ID_RADIUS1, radius1);
  this->SetQuantity(FEATURE_ID_RADIUS2, radius2);
  this->SetQuantity(FEATURE_ID_CIRCUMFERENCE, circumference);
  this->SetQuantity(FEATURE_ID_AREA, area);
	}

void mitk::PlanarEllipse::PrintSelf(std::ostream &os, itk::Indent indent) const
	{
  Superclass::PrintSelf(os, indent);
	}

bool mitk::PlanarEllipse::Equals(const mitk::PlanarFigure &other) const
{
  const mitk::PlanarEllipse *otherEllipse = dynamic_cast<const mitk::PlanarEllipse *>(&other);
  if (otherEllipse)
  {
    if (this->m_TreatAsCircle != otherEllipse->m_TreatAsCircle)
      return false;

    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

