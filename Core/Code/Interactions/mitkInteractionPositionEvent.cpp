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

#include "mitkInteractionPositionEvent.h"
#include <string>

mitk::InteractionPositionEvent::InteractionPositionEvent(mitk::BaseRenderer* baseRenderer,
    mitk::Point2D mousePosition,
    std::string eventClass) :
    InteractionEvent(baseRenderer, eventClass), m_PointerPosition(mousePosition)
{
  if (GetSender() != NULL)
  {
    m_WorldPosition = GetSender()->Map2DRendererPositionTo3DWorldPosition(m_PointerPosition);
  }
  else
  {
    m_WorldPosition[0] = 0;
    m_WorldPosition[1] = 0;
    m_WorldPosition[2] = 0;
    //MITK_WARN<< "Renderer not initialized. WorldCoordinates bogus!";
  }
}

mitk::Point2D mitk::InteractionPositionEvent::GetPointerPositionOnScreen()
{
  return m_PointerPosition;
}

mitk::Point3D mitk::InteractionPositionEvent::GetPositionInWorld()
{
  return m_WorldPosition;
}

bool mitk::InteractionPositionEvent::isEqual(InteractionEvent::Pointer)
{
  return true;
}

bool mitk::InteractionPositionEvent::IsSuperClassOf(InteractionEvent::Pointer baseClass)
{
  InteractionPositionEvent* event = dynamic_cast<InteractionPositionEvent*>(baseClass.GetPointer());
  if (event != NULL)
  {
    return true;
  }
  return false;
}

mitk::InteractionPositionEvent::~InteractionPositionEvent()
{
}

