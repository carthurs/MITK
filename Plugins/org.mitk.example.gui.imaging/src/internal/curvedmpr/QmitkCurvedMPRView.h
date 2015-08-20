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

#ifndef QMITK_CURVEDMPRVIEW_H_
#define QMITK_CURVEDMPRVIEW_H_

#include <QmitkAbstractView.h>
#include <QObject>

#include <string>

// pointset and interaction
#include <mitkPointInteractor.h>
#include "mitkPointSetDataInteractor.h"
#include <mitkPointSet.h>

#include <mitkInterpolateLinesFilter.h>
#include <mitkSliceNavigationController.h>

// filter
#include <mitkThinPlateSplineCurvedGeometry.h>
#include <mitkPlaneLandmarkProjector.h>
#include <mitkSphereLandmarkProjector.h>
#include <mitkPlaneFit.h>

#include "ui_QmitkCurvedMPRViewControls.h"

class QmitkCurvedMPRView : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkCurvedMPRView();
  QmitkCurvedMPRView( const QmitkCurvedMPRView& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }

  virtual ~QmitkCurvedMPRView();

protected slots:

  void PointSetChanged();

  void SigmaChanged(float);
  void SigmaSliderChanged(int);
  void AlwaysViewPointsToggled(bool on);
  void UpdateMappingRendererGeometry();
  void SetDisplayMode(bool);
  //void ConnectModeToggled(bool);
  void SetPlaneOrientation(int i);
  void CalculateCut();
  void CalculateResectionProposal();

  void updateImageLabel();

  bool eventFilter(QObject *, QEvent *) override;

protected:

  virtual void SetFocus();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void CreateConnections();

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

  void NodeRemoved(const mitk::DataNode* node) override;

  void RebuildMappingSurface();

  void AddWorkNodes();

  void AdjustPointVisibility();

  void OnPointSetNodeDeleted(const itk::EventObject&);

  Ui::QmitkCurvedMPRViewControls *m_Controls;

  // algorithms needed for the thin plate spline transoformation
  mitk::ThinPlateSplineCurvedGeometry::Pointer m_TPSGeometry;
  mitk::PlaneLandmarkProjector::Pointer m_PlaneLandmarkProjector;
  mitk::SphereLandmarkProjector::Pointer m_SphereLandmarkProjector;

  // storing the plane information
  mitk::PlaneGeometry::Pointer m_Plane;
  mitk::PlaneGeometry::Pointer m_XYPlane;
  mitk::PlaneGeometry::Pointer m_XZPlane;
  mitk::PlaneGeometry::Pointer m_YZPlane;

  // point set & interactions
  mitk::PointSetDataInteractor::Pointer m_PointSetInteractor;
  mitk::InterpolateLinesFilter::Pointer m_InterpolateLinesFilter;

  mitk::Mesh::Pointer m_PointSet;

  bool m_blockDataStorageChangeCallback;

  /** prevents multiplied interaction activation */
  bool m_InteractionEnabled;
  bool m_PointListNodeAdded;
  bool m_UseSphere;
  bool m_UsePrefitPlane;

  unsigned long m_MeshObserverTag;

  // nodes
  void setCurrentImageNode(mitk::DataNode* imageNode);

  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_ResultNode;
  mitk::DataNode::Pointer m_PointListNode;
  mitk::DataNode::Pointer m_ConnectedLineNode;

  // data storage
  mitk::DataStorage::Pointer m_DataStorage;

  mitk::BaseRenderer::Pointer m_MappingRenderer;

  QWidget* m_Parent;

  unsigned long m_AddCommandObserverTag;
  unsigned long m_MoveCommandObserverTag;
  unsigned long m_DelNodeCommandObserverTag;

  QImage GetTextureFromImageNode(mitk::DataNode* imageNode, mitk::BaseRenderer* renderer);
  std::unique_ptr<QLabel> m_ImageLabel;
  QPixmap m_CurrentImageSlice;
};

#endif
