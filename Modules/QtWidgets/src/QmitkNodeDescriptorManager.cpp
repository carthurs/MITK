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

#include "QmitkNodeDescriptorManager.h"
#include <memory>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateDataType.h>
#include <mitkProperties.h>

#include <QList>
#include <QSet>


QmitkNodeDescriptorManager* QmitkNodeDescriptorManager::GetInstance()
{
  static QmitkNodeDescriptorManager _Instance;
  return &_Instance;
}

void QmitkNodeDescriptorManager::Initialize()
{
  auto isImage = mitk::NodePredicateDataType::New("Image");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/Qmitk/Images_48.png"), isImage, this));

  auto isMultiComponentImage = mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateProperty::New("Image.Displayed Component"));
  this->AddDescriptor(new QmitkNodeDescriptor(tr("MultiComponentImage"), QString(": / Qmitk / Images_48.png"), isMultiComponentImage, this));
  // Adding "Image Masks"
  auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  auto isBinaryImage = mitk::NodePredicateAnd::New(isBinary, isImage);
  this->AddDescriptor(new QmitkNodeDescriptor(tr("ImageMask"), QString(":/Qmitk/Binaerbilder_48.png"), isBinaryImage, this));

  auto isLabelSetImage = mitk::NodePredicateDataType::New("LabelSetImage");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("LabelSetImage"), QString(":/Qmitk/LabelSetImage_48.png"), isLabelSetImage, this));
  // Adding "PointSet"
  auto isPointSet = mitk::NodePredicateDataType::New("PointSet");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PointSet"), QString(":/Qmitk/PointSet_48.png"), isPointSet, this));

  auto isSurface = mitk::NodePredicateDataType::New("Surface");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("Surface"), QString(":/Qmitk/Surface_48.png"), isSurface, this));

  auto isNotBinary = mitk::NodePredicateNot::New(isBinary);
  auto isNoneBinaryImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
  this->AddDescriptor(new QmitkNodeDescriptor(tr("NoneBinaryImage"), QString(":/Qmitk/Images_48.png"), isNoneBinaryImage, this));

}

void QmitkNodeDescriptorManager::AddDescriptor( QmitkNodeDescriptor* _Descriptor )
{
  _Descriptor->setParent(this);
  m_NodeDescriptors.push_back(_Descriptor);
}

void QmitkNodeDescriptorManager::RemoveDescriptor( QmitkNodeDescriptor* _Descriptor )
{
  int index = m_NodeDescriptors.indexOf(_Descriptor);

  if(index != -1)
  {
    m_NodeDescriptors.removeAt(index);
    _Descriptor->setParent(0);
    delete _Descriptor;
  }

}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor( const mitk::DataNode* _Node ) const
{
  QmitkNodeDescriptor* _Descriptor = m_UnknownDataNodeDescriptor;

  for(QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if((*it)->CheckNode(_Node))
      _Descriptor = *it;
  }

  return _Descriptor;
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor( const QString& _ClassName ) const
{
  QmitkNodeDescriptor* _Descriptor = 0;

  if( _ClassName == "Unknown" )
  {
      return m_UnknownDataNodeDescriptor;
  }
  else
  {
      for(QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
      {
        if((*it)->GetNameOfClass() == _ClassName)
          _Descriptor = *it;
      }
  }

  return _Descriptor;
}

QList<QmitkNodeDescriptor*> QmitkNodeDescriptorManager::GetAllDescriptors(const mitk::DataNode* _Node) const
{
  QList<QmitkNodeDescriptor*> _Descriptors;

  for (QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if ((*it)->CheckNode(_Node)) {
      _Descriptors << *it;
    }
  }

  return _Descriptors;
}

QList<QmitkNodeDescriptor*> QmitkNodeDescriptorManager::GetAllDescriptors(const QString& _ClassName) const
{
  QList<QmitkNodeDescriptor*> _Descriptors;

  if (_ClassName == "Unknown")
  {
    _Descriptors << m_UnknownDataNodeDescriptor;
  }
  else
  {
    for (QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
    {
      if ((*it)->GetNameOfClass() == _ClassName)
        _Descriptors << *it;
    }
  }

  return _Descriptors;
}


QList<QAction*> QmitkNodeDescriptorManager::GetActions( const mitk::DataNode* _Node ) const
{
  QList<QmitkNodeDescriptor*> descriptors;
  descriptors << m_UnknownDataNodeDescriptor;
  descriptors << GetAllDescriptors(_Node);
  QList<QAction*> actions;

  for (QList<QmitkNodeDescriptor*>::const_iterator it = descriptors.begin(); it != descriptors.end(); ++it)
  {
    actions.append((*it)->GetBatchActions());
    actions.append((*it)->GetActions());
    actions.append((*it)->GetSeparator());
  }
  actions.removeLast();

  return actions;
}

QList<QAction*> QmitkNodeDescriptorManager::GetActions( const QList<mitk::DataNode::Pointer> &_Nodes ) const
{
  QList<QAction*> actions;
  QSet<QmitkNodeDescriptor*> nodeDescriptorsSet;

  // find all descriptors for the nodes (unique)
  foreach (mitk::DataNode::Pointer node, _Nodes)
  {
    QList<QmitkNodeDescriptor*> currentNodeDescriptors = GetAllDescriptors(node);
    foreach(QmitkNodeDescriptor* descriptor, currentNodeDescriptors) {
      nodeDescriptorsSet.insert(descriptor);
    }
  }

  QList<QmitkNodeDescriptor*> nodeDescriptorsList;
  nodeDescriptorsList << m_UnknownDataNodeDescriptor;
  nodeDescriptorsList << nodeDescriptorsSet.toList();

  // add all actions for the found descriptors
  foreach(QmitkNodeDescriptor* descr, nodeDescriptorsList)
  {
    actions.append(descr->GetBatchActions());
    actions.append(descr->GetSeparator());
  }
  actions.removeLast();

  return actions;
}

QmitkNodeDescriptorManager::QmitkNodeDescriptorManager()
: m_UnknownDataNodeDescriptor(new QmitkNodeDescriptor("Unknown", QString(":/Qmitk/DataTypeUnknown_48.png"), 0, this))
{
  this->Initialize();
}

QmitkNodeDescriptorManager::~QmitkNodeDescriptorManager()
{
  //delete m_UnknownDataNodeDescriptor;
  //qDeleteAll(m_NodeDescriptors);
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetUnknownDataNodeDescriptor() const
{
  return m_UnknownDataNodeDescriptor;
}
