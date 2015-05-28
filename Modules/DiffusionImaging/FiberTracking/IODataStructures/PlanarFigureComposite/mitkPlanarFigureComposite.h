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

#ifndef _MITK_PLANARFIGURECOMPOSITE_H
#define _MITK_PLANARFIGURECOMPOSITE_H

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include <MitkFiberTrackingExports.h>
#include "mitkPlanarFigure.h"
#include "itkVectorContainer.h"
#include "mitkDataNode.h"


namespace mitk {

class MITKFIBERTRACKING_EXPORT PlanarFigureComposite : public BaseData
{

public:

    enum OperationType {
        AND,
        OR,
        NOT
    };

    mitkClassMacro(PlanarFigureComposite, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual void SetRequestedRegionToLargestPossibleRegion() override;
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    virtual bool VerifyRequestedRegion() override;
    virtual void SetRequestedRegion(const itk::DataObject*) override;

    // ///MUST HAVE IMPLEMENTATION//////
    bool SetControlPoint(unsigned int, const Point2D &, bool);

    unsigned int GetPlacementNumberOfControlPoints() const
    {
        return 0;
    }
    unsigned int GetMinimumNumberOfControlPoints() const
    {
        return 0;
    }
    unsigned int GetMaximumNumberOfControlPoints() const
    {
        return 0;
    }
    // /////////////////////////

    int getNumberOfChildren();
    mitk::BaseData::Pointer getChildAt(int);
    void addPlanarFigure(BaseData::Pointer);


    mitk::DataNode::Pointer getDataNodeAt(int);
    void addDataNode(mitk::DataNode::Pointer);
    void replaceDataNodeAt(int, mitk::DataNode::Pointer);

    // set if this compsition is AND, OR, NOT
    void setOperationType(OperationType);
    OperationType getOperationType() const;

protected:
    PlanarFigureComposite();
    virtual ~PlanarFigureComposite();

    PlanarFigureComposite(const Self& other);

private:
    OperationType m_compOperation;
};
}

#endif
