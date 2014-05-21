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


#include "mitkPlanarFigure.h"
#include "mitkGeometry2D.h"
#include "mitkProperties.h"
#include "mitkPlanarFigureOperation.h"
#include "mitkInteractionConst.h"
#include <mitkProportionalTimeGeometry.h>

#include "algorithm"


mitk::PlanarFigure::PlanarFigure()
: m_SelectedControlPoint(-1),
m_PreviewControlPointVisible(false),
m_FigurePlaced(false),
m_FigureFinalized(false),
m_Geometry2D(NULL),
m_PolyLineUpToDate(false),
m_HelperLinesUpToDate(false),
m_FeaturesUpToDate(false),
m_FeaturesMTime(0)
{


    m_HelperPolyLinesToBePainted = BoolContainerType::New();

    m_DisplaySize.first = 0.0;
    m_DisplaySize.second = 0;

    this->SetProperty("closed", mitk::BoolProperty::New(false));

    // Currently only single-time-step geometries are supported
    this->InitializeTimeGeometry(1);
}


mitk::PlanarFigure::~PlanarFigure()
{
}


mitk::PlanarFigure::PlanarFigure(const Self& other)
: BaseData(other),
m_ControlPoints(other.m_ControlPoints),
m_SelectedControlPoint(other.m_SelectedControlPoint),
m_PolyLines(other.m_PolyLines),
m_HelperPolyLines(other.m_HelperPolyLines),
m_HelperPolyLinesToBePainted(other.m_HelperPolyLinesToBePainted->Clone()),
m_PreviewControlPoint(other.m_PreviewControlPoint),
m_PreviewControlPointVisible(other.m_PreviewControlPointVisible),
m_FigurePlaced(other.m_FigurePlaced),
m_FigureFinalized(other.m_FigureFinalized),
m_Geometry2D(other.m_Geometry2D), // do not clone since SetGeometry2D() doesn't clone either
m_PolyLineUpToDate(other.m_PolyLineUpToDate),
m_HelperLinesUpToDate(other.m_HelperLinesUpToDate),
m_FeaturesUpToDate(other.m_FeaturesUpToDate),
m_Features(other.m_Features),
m_FeaturesMTime(other.m_FeaturesMTime),
m_DisplaySize(other.m_DisplaySize)
{
}


void mitk::PlanarFigure::SetGeometry2D(mitk::Geometry2D *geometry)
{
    this->SetGeometry(geometry);
    m_Geometry2D = dynamic_cast<Geometry2D *>(GetGeometry(0));//geometry;
}


const mitk::Geometry2D *mitk::PlanarFigure::GetGeometry2D() const
{
    return m_Geometry2D;
}


bool mitk::PlanarFigure::IsClosed() const
{
    mitk::BoolProperty* closed = dynamic_cast<mitk::BoolProperty*>(this->GetProperty("closed").GetPointer());
    if (closed != NULL)
    {
        return closed->GetValue();
    }
    return false;
}

void mitk::PlanarFigure::PlaceFigure(const mitk::Point2D& point)
{
    for (unsigned int i = 0; i < this->GetPlacementNumberOfControlPoints(); ++i)
    {
        AddControlPoint(this->ApplyControlPointConstraints(i, point), i);
    }

    m_FigurePlaced = true;
    SelectControlPoint(this->GetPlacementSelectedPointId());
    this->InvokeEvent(EndPlacementPlanarFigureEvent());
    this->Modified();
}

void mitk::PlanarFigure::CancelPlaceFigure()
{
    m_ControlPoints.resize(0);
    m_FigurePlaced = false;
    m_SelectedControlPoint = -1;
    this->InvokeEvent(CancelPlacementPlanarFigureEvent());
    this->Modified();
}

bool mitk::PlanarFigure::AddControlPoint(const mitk::Point2D& point, int position)
{
    // if we already have the maximum number of control points, do nothing
    if (GetNumberOfControlPoints() < this->GetMaximumNumberOfControlPoints())
    {
        // if position has not been defined or position would be the last control point, just append the new one
        // we also append a new point if we click onto the line between the first two control-points if the second control-point is selected
        // -> special case for PlanarCross
        if (position == -1 || position > (int)GetNumberOfControlPoints() - 1 || (position == 1 && m_SelectedControlPoint == 2))
        {
            m_ControlPoints.push_back(this->ApplyControlPointConstraints(m_ControlPoints.size(), point));
            SelectControlPoint(m_ControlPoints.size() - 1);
        }
        else
        {
            // insert the point at the given position and set it as selected point
            ControlPointListType::iterator iter = m_ControlPoints.begin() + position;
            m_ControlPoints.insert(iter, this->ApplyControlPointConstraints(position, point));
            SelectControlPoint(position);
        }

        // polylines & helperpolylines need to be repainted
        m_PolyLineUpToDate = false;
        m_HelperLinesUpToDate = false;
        m_FeaturesUpToDate = false;

        this->Modified();
        return true;
    }
    else
    {
        return false;
    }
}


bool mitk::PlanarFigure::SetControlPoint(unsigned int index, const Point2D& point, bool createIfDoesNotExist)
{
    bool controlPointSetCorrectly = false;
    if (createIfDoesNotExist)
    {
        if (GetNumberOfControlPoints() <= index)
        {
            m_ControlPoints.push_back(this->ApplyControlPointConstraints(index, point));
        }
        else
        {
            m_ControlPoints.at(index) = this->ApplyControlPointConstraints(index, point);
        }
        controlPointSetCorrectly = true;
    }
    else if (index < GetNumberOfControlPoints())
    {
        m_ControlPoints.at(index) = this->ApplyControlPointConstraints(index, point);
        controlPointSetCorrectly = true;
    }
    else
    {
        return false;
    }

    if (controlPointSetCorrectly)
    {
        m_PolyLineUpToDate = false;
        m_HelperLinesUpToDate = false;
        m_FeaturesUpToDate = false;
    }

    this->Modified();
    return controlPointSetCorrectly;
}


bool mitk::PlanarFigure::SetCurrentControlPoint(const Point2D& point)
{
    if ((m_SelectedControlPoint < 0) || (m_SelectedControlPoint >= (int)GetNumberOfControlPoints()))
    {
        return false;
    }

    return this->SetControlPoint(m_SelectedControlPoint, point, false);
}


unsigned int mitk::PlanarFigure::GetNumberOfControlPoints() const
{
    return m_ControlPoints.size();
}


bool mitk::PlanarFigure::SelectControlPoint(unsigned int index)
{
    if (index < this->GetNumberOfControlPoints())
    {
        m_SelectedControlPoint = index;
//         this->InvokeEvent(PlanarFigureSelectPointEvent());
        return true;
    }
    else
    {
        return false;
    }
}


bool mitk::PlanarFigure::DeselectControlPoint()
{
    bool wasSelected = (m_SelectedControlPoint != -1);

    m_SelectedControlPoint = -1;

//     if (wasSelected) {
//         this->InvokeEvent(PlanarFigureDeselectPointEvent());
//     }

    return wasSelected;
}

void mitk::PlanarFigure::SetPreviewControlPoint(const Point2D& point)
{
    m_PreviewControlPoint = point;
    m_PreviewControlPointVisible = true;
}

void mitk::PlanarFigure::ResetPreviewContolPoint()
{
    m_PreviewControlPointVisible = false;
}

mitk::Point2D mitk::PlanarFigure::GetPreviewControlPoint()
{
    return m_PreviewControlPoint;
}

bool mitk::PlanarFigure::IsPreviewControlPointVisible()
{
    return m_PreviewControlPointVisible;
}

mitk::Point2D mitk::PlanarFigure::GetControlPoint(unsigned int index) const
{
    if (index < GetNumberOfControlPoints())
    {
        return m_ControlPoints.at(index);
    }

    itkExceptionMacro(<< "GetControlPoint(): Invalid index!");
}


mitk::Point3D mitk::PlanarFigure::GetWorldControlPoint(unsigned int index) const
{
    Point3D point3D;
    if ((m_Geometry2D != NULL) && (index < GetNumberOfControlPoints()))
    {
        m_Geometry2D->Map(m_ControlPoints.at(index), point3D);
        return point3D;
    }

    itkExceptionMacro(<< "GetWorldControlPoint(): Invalid index!");
}


const mitk::PlanarFigure::PolyLineType
mitk::PlanarFigure::GetPolyLine(unsigned int index)
{
    mitk::PlanarFigure::PolyLineType polyLine;
    if (index > m_PolyLines.size() || !m_PolyLineUpToDate)
    {
        this->GeneratePolyLine();
        m_PolyLineUpToDate = true;
    }

    return m_PolyLines.at(index);;
}


const mitk::PlanarFigure::PolyLineType
mitk::PlanarFigure::GetPolyLine(unsigned int index) const
{
    return m_PolyLines.at(index);
}

void mitk::PlanarFigure::ClearPolyLines()
{
    for (std::vector<PolyLineType>::size_type i = 0; i < m_PolyLines.size(); i++)
    {
        m_PolyLines.at(i).clear();
    }
    m_PolyLineUpToDate = false;
}

const mitk::PlanarFigure::PolyLineType mitk::PlanarFigure::GetHelperPolyLine(unsigned int index,
    double mmPerDisplayUnit,
    unsigned int displayHeight)
{
    mitk::PlanarFigure::PolyLineType helperPolyLine;
    if (index < m_HelperPolyLines.size())
    {
        // m_HelperLinesUpToDate does not cover changes in zoom-level, so we have to check previous values of the
        // two parameters as well
        if (!m_HelperLinesUpToDate || m_DisplaySize.first != mmPerDisplayUnit || m_DisplaySize.second != displayHeight)
        {
            this->GenerateHelperPolyLine(mmPerDisplayUnit, displayHeight);
            m_HelperLinesUpToDate = true;

            // store these parameters to be able to check next time if somebody zoomed in or out
            m_DisplaySize.first = mmPerDisplayUnit;
            m_DisplaySize.second = displayHeight;
        }

        helperPolyLine = m_HelperPolyLines.at(index);
    }

    return helperPolyLine;
}

void mitk::PlanarFigure::ClearHelperPolyLines()
{
    for (std::vector<PolyLineType>::size_type i = 0; i < m_HelperPolyLines.size(); i++)
    {
        m_HelperPolyLines.at(i).clear();
    }
    m_HelperLinesUpToDate = false;
}

/** \brief Returns the number of features available for this PlanarFigure
* (such as, radius, area, ...). */
unsigned int mitk::PlanarFigure::GetNumberOfFeatures() const
{
    return m_Features.size();
}


const char *mitk::PlanarFigure::GetFeatureName(unsigned int index) const
{
    if (index < m_Features.size())
    {
        return m_Features[index].Name.c_str();
    }
    else
    {
        return NULL;
    }
}


const char *mitk::PlanarFigure::GetFeatureUnit(unsigned int index) const
{
    if (index < m_Features.size())
    {
        return m_Features[index].Unit.c_str();
    }
    else
    {
        return NULL;
    }
}


double mitk::PlanarFigure::GetQuantity(unsigned int index) const
{
    if (index < m_Features.size())
    {
        return m_Features[index].Quantity;
    }
    else
    {
        return 0.0;
    }
}


bool mitk::PlanarFigure::IsFeatureActive(unsigned int index) const
{
    if (index < m_Features.size())
    {
        return m_Features[index].Active;
    }
    else
    {
        return false;
    }
}

bool mitk::PlanarFigure::IsFeatureVisible(unsigned int index) const
{
    if (index < m_Features.size())
    {
        return m_Features[index].Visible;
    }
    else
    {
        return false;
    }
}

void mitk::PlanarFigure::SetFeatureVisible(unsigned int index, bool visible)
{
    if (index < m_Features.size())
    {
        m_Features[index].Visible = visible;
    }
}


void mitk::PlanarFigure::EvaluateFeatures()
{
    if (!m_FeaturesUpToDate || !m_PolyLineUpToDate)
    {
        if (!m_PolyLineUpToDate)
        {
            this->GeneratePolyLine();
        }

        this->EvaluateFeaturesInternal();

        m_FeaturesUpToDate = true;
    }
}


void mitk::PlanarFigure::UpdateOutputInformation()
{
    // Bounds are NOT calculated here, since the Geometry2D defines a fixed
    // frame (= bounds) for the planar figure.
    Superclass::UpdateOutputInformation();
    this->GetTimeGeometry()->Update();
}


void mitk::PlanarFigure::SetRequestedRegionToLargestPossibleRegion()
{
}


bool mitk::PlanarFigure::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}


bool mitk::PlanarFigure::VerifyRequestedRegion()
{
    return true;
}


void mitk::PlanarFigure::SetRequestedRegion(const itk::DataObject * /*data*/)
{

}


mitk::Point2D mitk::PlanarFigure::ApplyControlPointConstraints(unsigned int /*index*/, const Point2D& point)
{
    if (m_Geometry2D == NULL)
    {
        return point;
    }

    Point2D indexPoint;
    m_Geometry2D->WorldToIndex(point, indexPoint);

    BoundingBox::BoundsArrayType bounds = m_Geometry2D->GetBounds();
    if (indexPoint[0] < bounds[0]) { indexPoint[0] = bounds[0]; }
    if (indexPoint[0] > bounds[1]) { indexPoint[0] = bounds[1]; }
    if (indexPoint[1] < bounds[2]) { indexPoint[1] = bounds[2]; }
    if (indexPoint[1] > bounds[3]) { indexPoint[1] = bounds[3]; }

    Point2D constrainedPoint;
    m_Geometry2D->IndexToWorld(indexPoint, constrainedPoint);

    return constrainedPoint;
}


unsigned int mitk::PlanarFigure::AddFeature(const char *featureName, const char *unitName)
{
    unsigned int index = m_Features.size();

    Feature newFeature(featureName, unitName);
    m_Features.push_back(newFeature);

    return index;
}


void mitk::PlanarFigure::SetFeatureName(unsigned int index, const char *featureName)
{
    if (index < m_Features.size())
    {
        m_Features[index].Name = featureName;
    }
}


void mitk::PlanarFigure::SetFeatureUnit(unsigned int index, const char *unitName)
{
    if (index < m_Features.size())
    {
        m_Features[index].Unit = unitName;
    }
}


void mitk::PlanarFigure::SetQuantity(unsigned int index, double quantity)
{
    if (index < m_Features.size())
    {
        m_Features[index].Quantity = quantity;
    }
}


void mitk::PlanarFigure::ActivateFeature(unsigned int index)
{
    if (index < m_Features.size())
    {
        m_Features[index].Active = true;
    }
}


void mitk::PlanarFigure::DeactivateFeature(unsigned int index)
{
    if (index < m_Features.size())
    {
        m_Features[index].Active = false;
    }
}

void mitk::PlanarFigure::InitializeTimeGeometry(unsigned int timeSteps)
{
    mitk::Geometry2D::Pointer geometry2D = mitk::Geometry2D::New();
    geometry2D->Initialize();

    if (timeSteps > 1)
    {
        mitk::ScalarType timeBounds[] = { 0.0, 1.0 };
        geometry2D->SetTimeBounds(timeBounds);
    }

    // The geometry is propagated automatically to all time steps,
    // if EvenlyTimed is true...
    ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry2D, timeSteps);
    SetTimeGeometry(timeGeometry);
}


void mitk::PlanarFigure::PrintSelf(std::ostream& os, itk::Indent indent) const
{
    Superclass::PrintSelf(os, indent);
    os << indent << this->GetNameOfClass() << ":\n";

    if (this->IsClosed())
        os << indent << "This figure is closed\n";
    else
        os << indent << "This figure is not closed\n";
    os << indent << "Minimum number of control points: " << this->GetMinimumNumberOfControlPoints() << std::endl;
    os << indent << "Maximum number of control points: " << this->GetMaximumNumberOfControlPoints() << std::endl;
    os << indent << "Current number of control points: " << this->GetNumberOfControlPoints() << std::endl;
    os << indent << "Control points:" << std::endl;

    for (unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i)
    {
        //os << indent.GetNextIndent() << i << ": " << m_ControlPoints->ElementAt( i ) << std::endl;
        os << indent.GetNextIndent() << i << ": " << m_ControlPoints.at(i) << std::endl;
    }
    os << indent << "Geometry:\n";
    this->GetGeometry2D()->Print(os, indent.GetNextIndent());
}


unsigned short mitk::PlanarFigure::GetPolyLinesSize()
{
    if (!m_PolyLineUpToDate)
    {
        this->GeneratePolyLine();
        m_PolyLineUpToDate = true;
    }
    return m_PolyLines.size();
}


unsigned short mitk::PlanarFigure::GetHelperPolyLinesSize()
{
    return m_HelperPolyLines.size();
}


bool mitk::PlanarFigure::IsHelperToBePainted(unsigned int index)
{
    return m_HelperPolyLinesToBePainted->GetElement(index);
}


bool mitk::PlanarFigure::ResetOnPointSelect()
{
    return false;
}

void mitk::PlanarFigure::RemoveControlPoint(int index)
{
    if (index >= GetNumberOfControlPoints())
        return;

    if ((m_ControlPoints.size() - 1) < this->GetMinimumNumberOfControlPoints())
        return;

    if (index == -1) {
        RemoveLastControlPoint();
        return;
    }

    ControlPointListType::iterator iter;
    iter = m_ControlPoints.begin() + index;

    m_ControlPoints.erase(iter);

    m_PolyLineUpToDate = false;
    m_HelperLinesUpToDate = false;
    m_FeaturesUpToDate = false;
    this->Modified();
}

void mitk::PlanarFigure::RemoveLastControlPoint()
{
    RemoveControlPoint(GetNumberOfControlPoints() - 1);
}

void mitk::PlanarFigure::DeepCopy(Self::Pointer oldFigure)
{
    //DeepCopy only same types of planar figures
    //Notice to get typeid polymorph you have to use the *operator
    if (typeid(*oldFigure) != typeid(*this))
    {
        itkExceptionMacro(<< "DeepCopy(): Inconsistent type of source (" << typeid(*oldFigure).name() << ") and destination figure (" << typeid(*this).name() << ")!");
        return;
    }

    m_ControlPoints.clear();
    this->ClearPolyLines();
    this->ClearHelperPolyLines();

    // clone base data members
    SetPropertyList(oldFigure->GetPropertyList()->Clone());

    /// deep copy members
    m_FigurePlaced = oldFigure->m_FigurePlaced;
    m_SelectedControlPoint = oldFigure->m_SelectedControlPoint;
    m_FeaturesMTime = oldFigure->m_FeaturesMTime;
    m_Features = oldFigure->m_Features;

    //copy geometry 2D of planar figure
    Geometry2D::Pointer affineGeometry = oldFigure->m_Geometry2D->Clone();
    SetGeometry2D(affineGeometry.GetPointer());

    for (unsigned long index = 0; index < oldFigure->GetNumberOfControlPoints(); index++)
    {
        m_ControlPoints.push_back(oldFigure->GetControlPoint(index));
    }

    //After setting the control points we can generate the polylines
    this->GeneratePolyLine();
}

void mitk::PlanarFigure::SetNumberOfPolyLines(unsigned int numberOfPolyLines)
{
    m_PolyLines.resize(numberOfPolyLines);
}

void mitk::PlanarFigure::SetNumberOfHelperPolyLines(unsigned int numberOfHerlperPolyLines)
{
    m_HelperPolyLines.resize(numberOfHerlperPolyLines);
}


void mitk::PlanarFigure::AppendPointToPolyLine(unsigned int index, PolyLineElement element)
{
    if (index < m_PolyLines.size())
    {
        m_PolyLines.at(index).push_back(element);
        m_PolyLineUpToDate = false;
    }
    else
    {
        MITK_ERROR << "Tried to add point to PolyLine " << index + 1 << ", although only " << m_PolyLines.size() << " exists";
    }
}

void mitk::PlanarFigure::AppendPointToHelperPolyLine(unsigned int index, PolyLineElement element)
{
    if (index < m_HelperPolyLines.size())
    {
        m_HelperPolyLines.at(index).push_back(element);
        m_HelperLinesUpToDate = false;
    }
    else
    {
        MITK_ERROR << "Tried to add point to HelperPolyLine " << index + 1 << ", although only " << m_HelperPolyLines.size() << " exists";
    }
}

void mitk::PlanarFigure::ExecuteOperation(Operation* operation)
{
    mitkCheckOperationTypeMacro(PlanarFigureOperation, operation, planarFigureOp);

    switch (operation->GetOperationType())
    {
    case OpNOTHING:
        break;

    case OpINSERT://inserts the point at the given position and selects it.
        this->AddControlPoint(planarFigureOp->GetPoint(), planarFigureOp->GetIndex());
        break;

    case OpMOVE://moves the point given by index
        this->SetControlPoint(planarFigureOp->GetIndex(), planarFigureOp->GetPoint());
        break;

    case OpREMOVE://removes the point at given by position
        this->RemoveControlPoint(planarFigureOp->GetIndex());
        break;

    case OpSELECTPOINT://select the given point
        this->SelectControlPoint(planarFigureOp->GetIndex());
        break;

    case OpDESELECTPOINT://unselect the given point
        this->DeselectControlPoint();
        break;

    case OpADD:
        this->PlaceFigure(planarFigureOp->GetPoint());
        break;

    case OpUNDOADD:
        this->CancelPlaceFigure();
        break;

    case OpCLOSECELL:
        this->SetFigureFinalized(true);
        break;

    case OpOPENCELL:
        this->SetFigureFinalized(false);
        break;

    default:
        itkWarningMacro("mitkPlanarFigure could not understrand the operation. Please check!");
        break;
    }
}
