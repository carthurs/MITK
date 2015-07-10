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

#ifndef QmitkExtFileSaveProjectAction_H_
#define QmitkExtFileSaveProjectAction_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>

#include <org_mitk_gui_qt_ext_Export.h>

#include <berrySmartPointer.h>
#include <mitkSceneIO.h>

namespace berry {
struct IWorkbenchWindow;
}

class MITK_QT_COMMON_EXT_EXPORT QmitkExtFileSaveProjectAction : public QAction
{
  Q_OBJECT

public:

  QmitkExtFileSaveProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window, mitk::SceneIO::Pointer sceneIO, bool saveAs);
  QmitkExtFileSaveProjectAction(berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO, bool saveAs);

protected slots:

  void Run();

signals:
  void projectSaved(QString fileName);

private:

  void Init(berry::IWorkbenchWindow* window);

  berry::IWorkbenchWindow* m_Window;
  mitk::SceneIO::Pointer m_SceneIO;
  bool m_SaveAs;
};


#endif /*QmitkExtFileSaveProjectAction_H_*/
