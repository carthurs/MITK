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

#include "QmitkExtFileOpenProjectAction.h"

#include "internal/QmitkCommonExtPlugin.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

#include <mitkSceneIO.h>
#include <mitkProgressBar.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkWorkbenchUtil.h>

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include "berryPlatform.h"


// DUPLICATED FROM QmitkFileOpenAction
static berry::IPreferences::Pointer GetPreferences() 
{
    berry::IPreferencesService::Pointer prefService
        = berry::Platform::GetServiceRegistry()
        .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
    if (prefService.IsNotNull())
    {
        return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(0);
}

static QString getLastFileOpenPath() 
{
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
        return QString::fromStdString(prefs->Get("LastProjectFileOpenPath", ""));
    }
    return QString();
}

static void setLastFileOpenPath(const QString& path) 
{
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
        prefs->Put("LastProjectFileOpenPath", path.toStdString());
        prefs->Flush();
    }
}
// end DUPLICATED


QmitkExtFileOpenProjectAction::QmitkExtFileOpenProjectAction(berry::IWorkbenchWindow::Pointer window, mitk::SceneIO::Pointer sceneIO)
: QAction(0)
, m_SceneIO(sceneIO)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Open Project");
  this->setToolTip("Open an .mitk project file");
  m_Window = window;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkExtFileOpenProjectAction::Run(QString fName)
{
  try
  {
    /**
     * @brief stores the last path of last saved file
     */
    mitk::IDataStorageReference::Pointer dsRef;

    {
      ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
      mitk::IDataStorageService* dss = 0;
      ctkServiceReference dsServiceRef = context->getServiceReference<mitk::IDataStorageService>();
      if (dsServiceRef)
      {
        dss = context->getService<mitk::IDataStorageService>(dsServiceRef);
      }

      if (!dss)
      {
        QString msg = "IDataStorageService service not available. Unable to open files.";
        MITK_WARN << msg.toStdString();
        QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
        return;
      }

      // Get the active data storage (or the default one, if none is active)
      dsRef = dss->GetDataStorage();
      context->ungetService(dsServiceRef);
    }

    mitk::DataStorage::Pointer storage = dsRef->GetDataStorage();

    QString fileName = fName;

    if (fileName.isEmpty()) {
        QString dialogTitle = "Open MITK Scene (%1)";
        fileName = QFileDialog::getOpenFileName(NULL,
            dialogTitle.arg(dsRef->GetLabel()),
            getLastFileOpenPath(),
            "MITK scene files (*.mitk)",
            NULL);
    }

    if (fileName.isEmpty())
        return;

    // Close project
    //check if we got the default datastorage and if there is anything else then helper object in the storage
    if (dsRef->IsDefault() ||
        !storage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true))))->empty())
    {
        /* Ask, if the user is sure about that */
        QString msg = "Are you sure that you want to close the current project (%1)?\nThis will remove all data objects.";
        if (QMessageBox::question(NULL, "Remove all data?", msg.arg(m_SceneIO->GetLoadedProjectFileName().empty() ? dsRef->GetLabel() : QString::fromStdString(m_SceneIO->GetLoadedProjectFileName())),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        {
            return;
        }
    }

    // remember the location
    setLastFileOpenPath(fileName);

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

    /* Build list of nodes that should be saved */
    if ( !m_SceneIO->LoadScene(fileName.toStdString(), storage, true) || m_SceneIO->GetLoadedProjectFileName().empty())
    {
      QMessageBox::information(NULL,
                               "Scene opening",
                               "Scene could not be loaded. Please check the log.",
                               QMessageBox::Ok);
      emit projectOpenFailed(fileName);
    }
    else {
        emit projectOpened(fileName);
    }

    mitk::WorkbenchUtil::ReinitAfterLoadFiles(m_Window, true, dsRef, true);
    mitk::ProgressBar::GetInstance()->Progress(2);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during scene loading: " << e.what();
  }
}
