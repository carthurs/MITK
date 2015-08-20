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

#include "QmitkCurvedMPRView.h"

#include <mitkSurface.h>
#include <mitkPointSetVtkMapper2D.h>
#include <mitkPointSetVtkMapper3D.h>

#include "mitkGlobalInteraction.h"

#include "mitkNodePredicateDataType.h"
#include "mitkImage.h"

#include <itkCommand.h>

#include <mitkImageVtkMapper2D.h>
#include <mitkWeakPointerProperty.h>
#include <vtkMitkLevelWindowFilter.h>

#include <vtkNew.h>
#include <vtkImageResample.h>

#include <array>

const std::string QmitkCurvedMPRView::VIEW_ID = "org.mitk.views.curvedmpr";

QmitkCurvedMPRView::QmitkCurvedMPRView()
  : QmitkAbstractView(),
    m_Controls(NULL), m_TPSGeometry(NULL), m_PlaneLandmarkProjector(NULL),
    m_SphereLandmarkProjector(NULL), m_ResultNode(NULL), m_DataStorage(NULL),
    m_PointSet(NULL), m_PointSetInteractor(NULL), m_blockDataStorageChangeCallback(false),
    m_InteractionEnabled(false), m_PointListNodeAdded(false), m_MappingRenderer(NULL),
    m_Parent(NULL)
{
  this->m_AddCommandObserverTag = 0;
  this->m_MoveCommandObserverTag = 0;
  this->m_DelNodeCommandObserverTag = 0;

  m_Plane  = NULL;
  m_XYPlane = mitk::PlaneGeometry::New();
  m_XZPlane = mitk::PlaneGeometry::New();
  m_YZPlane = mitk::PlaneGeometry::New();

  m_TPSGeometry = mitk::ThinPlateSplineCurvedGeometry::New();

  m_PlaneLandmarkProjector = mitk::PlaneLandmarkProjector::New();
  m_SphereLandmarkProjector = mitk::SphereLandmarkProjector::New();

  m_DataStorage = this->GetDataStorage();

  // Create PointSetData Interactor
  m_PointSetInteractor = mitk::PointSetDataInteractor::New();
  // Load the according state machine for regular point set interaction
  m_PointSetInteractor->LoadStateMachine("PointSet.xml");
  // Set the configuration file that defines the triggers for the transitions
  m_PointSetInteractor->SetEventConfig("PointSetConfig.xml");
}

QmitkCurvedMPRView::~QmitkCurvedMPRView()
{
  /** observer to be removed (if created)
    this->m_AddCommandObserverTag = 0;
    this->m_MoveCommandObserverTag = 0;
    this->m_DelNodeCommandObserverTag = 0;
  */
    setCurrentImageNode(nullptr);

}

void QmitkCurvedMPRView::CreateQtPartControl(QWidget *parent)
{
  if(m_Controls==NULL)
  {
    m_Controls = new Ui::QmitkCurvedMPRViewControls;
    m_Controls->setupUi(parent);
    m_Parent = parent;
    this->CreateConnections();


    auto renderWindow = new QmitkRenderWindow(parent, "reslicer", nullptr, mitk::RenderingManager::GetInstance());
    renderWindow->GetRenderer()->SetDataStorage(GetDataStorage());
    renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    renderWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_Controls->vboxLayout->addWidget(renderWindow);
    m_MappingRenderer = renderWindow->GetRenderer();
    renderWindow->hide();

    m_ImageLabel = std::make_unique<QLabel>();
    m_ImageLabel->installEventFilter(this);
  }
}

void QmitkCurvedMPRView::CreateConnections()
{
  //connect( m_Controls->SigmaSlider, SIGNAL(SigmaChanged(float)), this, SLOT(SigmaChanged(float)));
  connect( m_Controls->SigmaSlider, SIGNAL(valueChanged(int)), this, SLOT(SigmaSliderChanged(int)));
  connect( m_Controls->showPoints, SIGNAL(toggled(bool)), this, SLOT(AlwaysViewPointsToggled(bool)));
  //connect( m_Controls->rb3D, SIGNAL(clicked()), this, SLOT( SetDisplayMode(true)) );
  //connect( m_Controls->rb2D, SIGNAL(clicked()), this, SLOT( SetDisplayMode(false)) );

  connect( m_Controls->PlaneComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetPlaneOrientation(int)));
}

/*void QmitkCurvedMPRView::Activated()
{
  QmitkAbstractView::Activated();

  if(m_MappingRenderer.IsNull())
  {
    m_MappingRenderer = mitk::BaseRenderer::GetInstance( this->GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow() );
  }
}
*/
void QmitkCurvedMPRView::SetFocus()
{
//    if (m_MappingRenderer.IsNull() && this->GetRenderWindowPart())
//  {
//    m_MappingRenderer = mitk::BaseRenderer::GetInstance( this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() );
//  }
}

void QmitkCurvedMPRView::SigmaChanged(float f)
{

}

void QmitkCurvedMPRView::SigmaSliderChanged(int i)
{
    m_TPSGeometry->SetSigma(i / 100.0);
    RebuildMappingSurface();
}

void QmitkCurvedMPRView::AlwaysViewPointsToggled(bool on)
{

}

void QmitkCurvedMPRView::UpdateMappingRendererGeometry()
{


  if( m_MappingRenderer.IsNull() )
  {
    return;
  }

  // check which renderer is selected as active and
  // set the mapperID accordingly
  if(true || m_Controls->rb2D->isChecked() )
  {
    m_MappingRenderer->SetMapperID(mitk::BaseRenderer::Standard2D);
  }
  else
  {
    m_MappingRenderer->SetMapperID(mitk::BaseRenderer::Standard3D);
  }

  MITK_INFO("curvedmpr.update.setwg") << " before";
  //MITK_INFO("curvedmpr.update.setwg") << "TS " << m_TPSGeometry->GetLandmarkProjector()->GetTimeStamp();
  // select if curved geometry is to be displayed
  if(m_Controls->rbCurved->isChecked() && m_TPSGeometry->IsValid())
  {
    MITK_INFO("curvedmpr.update.setwg") << " to TPS";
    m_MappingRenderer->SetWorldGeometry3D( m_TPSGeometry );
    m_Controls->crossSectionNavigator->setEnabled(false);
  }
  // TODO: add cross section


  // force update
  m_MappingRenderer->ForceImmediateUpdate();
}

void QmitkCurvedMPRView::SetDisplayMode(bool displayIn3D)
{

}

void QmitkCurvedMPRView::SetPlaneOrientation(int i)
{
  try
  {
    /*
        PlaneComboBox->insertItems(0, QStringList()
         << QApplication::translate("QmitkCurvedMPRViewControls", "xy_plane", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("QmitkCurvedMPRViewControls", "xz_plane", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("QmitkCurvedMPRViewControls", "yz_plane", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("QmitkCurvedMPRViewControls", "sphere", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("QmitkCurvedMPRViewControls", "prefit plane", 0, QApplication::UnicodeUTF8)
        );
     */
    switch(i)
    {
    case 1:
      m_Plane = static_cast<mitk::PlaneGeometry*>(m_XZPlane->Clone().GetPointer());
      break;
    case 2:
      m_Plane = static_cast<mitk::PlaneGeometry*>(m_YZPlane->Clone().GetPointer());
      break;
    case 3:
      m_Plane = NULL;
      break;
    case 4:
      MITK_INFO << "Selecting PrefitPlane \n";
      m_Plane = NULL;
      // omitting brake command, since the plane will be assigned by the default case
    default:
      // the XY plane is the default value, also the value for the case 0 and also case 4
      m_Plane = static_cast<mitk::PlaneGeometry*>(m_XYPlane->Clone().GetPointer());
      break;
    }

    // set the bool values for 'using sphere' and 'using prefit plane'
    m_UseSphere = (i==3);
    m_UsePrefitPlane = (i==4);

    // TODO: Add member checking
    this->RebuildMappingSurface();

    if( m_Controls->rbCurved->isEnabled() && m_TPSGeometry->IsValid() )
    {
      UpdateMappingRendererGeometry();
      m_MappingRenderer->GetDisplayGeometry()->Fit();
      //this->GetActiveStdMultiWidget()->RequestUpdate();

      mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
      renderWindowPart->RequestUpdate();
    }
  }
  catch (std::exception &e)
  {
    HandleException(e.what(), m_Parent, true);
  }
}

void QmitkCurvedMPRView::CalculateCut()
{

}

void QmitkCurvedMPRView::CalculateResectionProposal()
{

}

void QmitkCurvedMPRView::updateImageLabel()
{
    m_ImageLabel->setPixmap(m_CurrentImageSlice.scaled(m_ImageLabel->width(), m_ImageLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_ImageLabel->show();
}

bool QmitkCurvedMPRView::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_ImageLabel.get() && e->type() == QEvent::Resize) {
        updateImageLabel();
    }
    return false;
}

void QmitkCurvedMPRView::OnPointSetNodeDeleted(const itk::EventObject &e)
{
/*  if(this->m_DelNodeCommandObserverTag ) m_PointListNode->RemoveObserver( this->m_PointListNode );
  this->m_DelNodeCommandObserverTag = 0;*/
}



void QmitkCurvedMPRView::AddWorkNodes()
{
}

void QmitkCurvedMPRView::AdjustPointVisibility()
{
  try
  {
    bool visibility = true;//m_Controls->showPoints->isOn();
    m_PointListNode->SetVisibility(visibility);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  catch( std::exception &e)
  {
    HandleException( e.what(), m_Parent, true);
  }
}

void QmitkCurvedMPRView::RebuildMappingSurface()
{
  MITK_INFO("curvedmpr.rebuild.surface") << "Enter";
  if( m_PointListNode.IsNull())
  {
    MITK_WARN << "PointListNode is NULL ";
    return;
  }

  // TODO better solution -> use the m_Mesh to get the point count information
  mitk::PointSet::Pointer data = dynamic_cast<mitk::PointSet*>(m_PointListNode->GetData());
  if(data.IsNull() || data->GetSize() < 2)
  {
    MITK_WARN << "Not enough points to create a plane. ";
    return;
  }

  try
  {
    m_TPSGeometry->SetTargetLandmarks(m_PointSet->GetPointSet()->GetPoints());

    if (m_UsePrefitPlane)
    {
        mitk::PlaneFit::Pointer myplanefit;
        myplanefit = mitk::PlaneFit::New();
        myplanefit->SetInput(m_PointSet);
        myplanefit->Update();
        m_Plane = myplanefit->GetPlaneGeometry();

        m_Plane->SetOrigin(m_Plane->GetOrigin() - m_Plane->GetAxisVector(0) * 200 - m_Plane->GetAxisVector(1) * 200);
        m_Plane->SetFloatBounds(std::array<float, 6>{0,400,0,400,0,0}.data());
        m_Plane->SetReferenceGeometry(m_XYPlane->GetReferenceGeometry());
    }

    if(!m_UseSphere)
    {
      MITK_INFO("curvedmpr.rebuild.surface") << "Befor projection";
      m_TPSGeometry->SetLandmarkProjector(m_PlaneLandmarkProjector);
      m_PlaneLandmarkProjector->SetProjectionPlane(m_Plane);
      MITK_INFO("curvedmpr.rebuild.surface") << "After projection";
    }
    else
    {
      m_TPSGeometry->SetLandmarkProjector(m_SphereLandmarkProjector);
    }

    //   m_Controls->GetSavePoints()->setEnabled(true);
    //   m_Controls->GetConnectMode()->setEnabled(true);
    m_Controls->rbCurved->setEnabled(true);
    m_Controls->rb2D->setEnabled(true);
    m_Controls->rbCrossSection->setEnabled(true);

    MITK_INFO("curvedmpr.rebuild.surface") << "BComp geom";
    m_TPSGeometry->ComputeGeometry();
    MITK_INFO("curvedmpr.rebuild.surface") << "AComp geom";
    //m_TPSGeometry->SetOversampling(m_Controls->sbOversampling->value());

//    if( m_MappingRenderer.IsNull() )
//    {
//      m_MappingRenderer = mitk::BaseRenderer::GetInstance( this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() );
//    }

    if(m_ResultNode.IsNull())
    {
      // tell m_MappingRenderer to use the new geometry
      //mitk::BaseRenderer::Pointer mappingRenderer = mitk::BaseRenderer::GetInstance( this->GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow());
      m_MappingRenderer->GetDisplayGeometry()->Fit();

      mitk::DataNode::Pointer m_TPSGeometryDataNode = m_MappingRenderer->GetCurrentWorldPlaneGeometryNode();
      m_TPSGeometryDataNode->SetIntProperty("xresolution", 200);
      m_TPSGeometryDataNode->SetIntProperty("yresolution", 200);
      m_TPSGeometryDataNode->SetProperty( "name", mitk::StringProperty::New( "curved plane" ) );

      // exclude extent of this plane when calculating DataTree bounding box
      m_TPSGeometryDataNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));

      m_blockDataStorageChangeCallback=true;
      //mitk::DataTreeIteratorClone child=m_IteratorOnImageToBeMapped.GetPointer();
      //child->GoToChild();
      //child->Add(m_TPSGeometryDataNode);
      m_ResultNode = m_TPSGeometryDataNode;
      m_ResultNode->SetVisibility(m_Controls->showCurvedReformat->isChecked(), m_MappingRenderer);
      m_DataStorage->Add(m_ResultNode);

      m_blockDataStorageChangeCallback=false;
    }
    else
    {
      m_ResultNode->GetData()->Modified();
      m_MappingRenderer->ForceImmediateUpdate();

//      auto geometry = mitk::ThinPlateSplineCurvedGeometry::New();
//      geometry->SetLandmarkProjector(m_PlaneLandmarkProjector);
//      geometry->SetTargetLandmarks(m_PointSet->GetPointSet()->GetPoints());
//      geometry->SetFrameGeometry(m_TPSGeometry->GetReferenceGeometry());
//      geometry->SetReferenceGeometry(m_TPSGeometry->GetReferenceGeometry());
//      geometry->ComputeGeometry();
//
//
//      auto renderWindow = new QmitkRenderWindow(nullptr, "reslicer", nullptr, mitk::RenderingManager::GetInstance());
//      renderWindow->GetRenderer()->SetDataStorage(GetDataStorage());
//      renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
//      renderWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//
//      renderWindow->GetSliceNavigationController()->SetDefaultViewDirection(
//          mitk::SliceNavigationController::Frontal);
//
//
//      renderWindow->GetRenderer()->SetWorldGeometry3D(geometry);
//
//      renderWindow->show();

    }
 
    m_CurrentImageSlice = QPixmap::fromImage(GetTextureFromImageNode(m_ImageNode, m_MappingRenderer));
    updateImageLabel();
    //m_imageLabel->adjustSize();

    /*
    m_InterpolateLinesFilter->SetGeometryForInterpolation(m_TPSGeometry);
    m_InterpolateLinesFilter->Modified();
    m_InterpolateLinesFilter->Update(); //recalculate length!
*/
    /*  QString slength;
    slength.setNum(m_InterpolateLinesFilter->GetLength(),'f',2);
    m_Controls->GetLengthOutput()->setText(slength);
*/

  }
  catch (std::exception& e)
  {
    HandleException( e.what(), m_Parent, true );
  }
}

void QmitkCurvedMPRView::PointSetChanged()
{
  try
  {
    RebuildMappingSurface();

    UpdateMappingRendererGeometry();
    if(m_TPSGeometry->IsValid())
    {
      m_MappingRenderer->GetDisplayGeometry()->Fit();

      mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(m_MappingRenderer->GetRenderWindow());
//
//      mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
//      renderWindowPart->RequestUpdate();
    }
  }
  catch (std::exception& e)
  {
    HandleException( e.what(), m_Parent, true );
  }

}

void QmitkCurvedMPRView::OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
                                            const QList<mitk::DataNode::Pointer> &nodes)
{
  MITK_INFO << "Entering OnSelCh";

  // do nothing if interaction explicitely blocked
  if(m_blockDataStorageChangeCallback)
    return;

  if(nodes.empty())
  {
    MITK_INFO << "no slection";
    return;
  }

  // create NodePredicate for image nodes
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage
      = mitk::TNodePredicateDataType<mitk::Image>::New();

  // get the topmost image node from data storage
  mitk::DataNode::Pointer selectedNode = nodes.at(0); //m_DataStorage->GetNode(isImage);

  MITK_INFO << "Selected node" << selectedNode->GetName();

  // return if no image present
  if(selectedNode.IsNull())
    return;

  // node not empty
  if( selectedNode->GetData() != NULL )
  {
    // access the original image
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(selectedNode->GetData());

    // proceed only if the cast was successful
    if( image.IsNotNull() )
    {
      // prevent proceeding on empty images
      if( image->IsInitialized() && image->IsVolumeSet() )
      {
         setCurrentImageNode(selectedNode);
      }
    }
  }
}

void QmitkCurvedMPRView::NodeRemoved(const mitk::DataNode* node)
{
    if (node == m_ImageNode) {
        setCurrentImageNode(nullptr);
    }
}

void QmitkCurvedMPRView::setCurrentImageNode(mitk::DataNode* imageNode)
{
    m_ImageNode = imageNode;

    if (!imageNode) {
        if (m_PointSet) {
            if (this->m_AddCommandObserverTag)  m_PointSet->RemoveObserver(this->m_AddCommandObserverTag);
            if (this->m_MoveCommandObserverTag)  m_PointSet->RemoveObserver(this->m_MoveCommandObserverTag);
            if (this->m_DelNodeCommandObserverTag) m_PointListNode->RemoveObserver(this->m_DelNodeCommandObserverTag);
        }

        m_PointListNode = nullptr;
        m_PointSet = nullptr;
    }
    else {
        auto pointSetNodes = GetDataStorage()->GetDerivations(imageNode, mitk::TNodePredicateDataType<mitk::Mesh>::New());
        if (pointSetNodes->empty()) {
            mitk::IntProperty::Pointer layerProp = mitk::IntProperty::New(1);

            // create a new point list node and name it
            m_PointListNode = mitk::DataNode::New();
            m_PointListNode->SetName("Control points for Thin plate spline");

            m_PointSet = mitk::Mesh::New();
            // set the DataNode (which already is added to the DataStorage
            m_PointListNode->SetData(m_PointSet);

            // set the data, layer, color and mapper for the point set node
            m_PointListNode->SetMapper(1, mitk::PointSetVtkMapper2D::New());
            m_PointListNode->SetMapper(2, mitk::PointSetVtkMapper3D::New());
            m_PointListNode->SetProperty("layer", layerProp);
            m_PointListNode->SetVisibility(true);
            m_PointListNode->SetColor(1.0, 1.0, 0.0);
            m_PointListNode->SetProperty("helper object", mitk::BoolProperty::New(true));
            this->AdjustPointVisibility();

            GetDataStorage()->Add(m_PointListNode, imageNode);
        }
        else {
            m_PointListNode = (*pointSetNodes)[0];
            m_PointSet = static_cast<mitk::Mesh*>(m_PointListNode->GetData());
        }

        auto image = static_cast<mitk::Image*>(imageNode->GetData());

        image->UpdateOutputInformation();

        const mitk::BaseGeometry *imageGeometry = image->GetUpdatedGeometry();

        // initialize the standard planes
        m_XYPlane->InitializeStandardPlane(imageGeometry, mitk::PlaneGeometry::Axial);
        m_YZPlane->InitializeStandardPlane(imageGeometry, mitk::PlaneGeometry::Sagittal);
        m_XZPlane->InitializeStandardPlane(imageGeometry, mitk::PlaneGeometry::Frontal);

        // propagate the orientation selected in the combobox
        SetPlaneOrientation(m_Controls->PlaneComboBox->currentIndex());

        this->m_TPSGeometry->SetFrameGeometry(imageGeometry);
        this->m_TPSGeometry->SetReferenceGeometry(const_cast<mitk::BaseGeometry*>(imageGeometry));

        itk::SimpleMemberCommand<QmitkCurvedMPRView>::Pointer point_add_command =
            itk::SimpleMemberCommand<QmitkCurvedMPRView>::New();
        point_add_command->SetCallbackFunction(this, &QmitkCurvedMPRView::PointSetChanged);
        m_AddCommandObserverTag = m_PointSet->AddObserver(itk::ModifiedEvent(), point_add_command);
        //m_MoveCommandObserverTag = m_PointSet->AddObserver( mitk::PointSetMoveEvent(), point_add_command );
    }

    m_PointSetInteractor->SetDataNode(m_PointListNode);
    m_Controls->pointListWidget->SetPointSetNode(m_PointListNode);
}

QImage QmitkCurvedMPRView::GetTextureFromImageNode(mitk::DataNode* node, mitk::BaseRenderer* renderer)
{
    //we need to get the information from the 2D mapper to render the texture on the 3D plane
    mitk::ImageVtkMapper2D *imageMapper = dynamic_cast<mitk::ImageVtkMapper2D *>(node->GetMapper(1)); //GetMapper(1) provides the 2D mapper for the data node

                                                                                            //if there is a 2D mapper, which is not the standard image mapper...
    
    if (!imageMapper && node->GetMapper(1))
    { //... check if it is the composite mapper
        std::string cname(node->GetMapper(1)->GetNameOfClass());
        if (!cname.compare("CompositeMapper")) //string.compare returns 0 if the two strings are equal.
        {
            //get the standard image mapper.
            //This is a special case in MITK and does only work for the CompositeMapper.
            imageMapper = dynamic_cast<mitk::ImageVtkMapper2D*>(node->GetMapper(3));
        }
    }

    if ((node->IsVisible(renderer)) && imageMapper)
    {
//         mitk::WeakPointerProperty::Pointer rendererProp =
//             dynamic_cast<mitk::WeakPointerProperty *>(GetDataNode()->GetPropertyList()->GetProperty("renderer"));
// 
//         if (rendererProp.IsNotNull())
//         {
        mitk::BaseRenderer::Pointer planeRenderer = renderer;// dynamic_cast<mitk::BaseRenderer *>(rendererProp->GetWeakPointer().GetPointer());
            // Retrieve and update image to be mapped
            const mitk::ImageVtkMapper2D::LocalStorage* localStorage = imageMapper->GetLocalStorage(planeRenderer);

            if (planeRenderer.IsNotNull())
            {
                // perform update of imagemapper if needed (maybe the respective 2D renderwindow is not rendered/update before)
                imageMapper->Update(planeRenderer);

                auto image = localStorage->m_LevelWindowFilter->GetOutput();

                vtkNew<vtkImageResample> resampler;
                resampler->SetInputData(image);
                auto spacing = std::min(image->GetSpacing()[0], image->GetSpacing()[1]);
                resampler->SetAxisOutputSpacing(0, spacing);
                resampler->SetAxisOutputSpacing(1, spacing);
                resampler->Update();

                auto imageData = resampler->GetOutput();

                QImage result(imageData->GetExtent()[1] - imageData->GetExtent()[0], 
                    imageData->GetExtent()[3] - imageData->GetExtent()[2], QImage::Format_RGB888);

                for (auto y = 0; y < result.height(); ++y) {
                    for (auto x = 0; x < result.width(); ++x) {
                        QColor color;
                        color.setRed((int)imageData->GetScalarComponentAsFloat(x + imageData->GetExtent()[0], y + imageData->GetExtent()[2], 0, 0));
                        color.setGreen((int)imageData->GetScalarComponentAsFloat(x + imageData->GetExtent()[0], y + imageData->GetExtent()[2], 0, 1));
                        color.setBlue((int)imageData->GetScalarComponentAsFloat(x + imageData->GetExtent()[0], y + imageData->GetExtent()[2], 0, 2));

                        result.setPixel(x, y, color.rgb());
                    }
                }

                return result;
            }
//         }
    }

    return QImage();
}
