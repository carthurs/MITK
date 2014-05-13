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


#ifndef _MITK_PLANAR_POINT_H_
#define _MITK_PLANAR_POINT_H_

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>


namespace mitk
{

/**
 * \brief Implementation of PlanarFigure representing a circle
 * through two control points
 */
class MitkPlanarFigure_EXPORT PlanarPoint : public PlanarFigure
{
public:
  mitkClassMacro( PlanarPoint, PlanarFigure );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();

  bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist = false );

  /** \brief Circle has 2 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 1;
  }


  /** \brief Circle has 2 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 1;
  }

  protected:
  PlanarPoint();
  virtual ~PlanarPoint();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine() {}

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) {}

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal() {}

  private:

};

} // namespace mitk

#endif //_MITK_PLANAR_POINT_H_
