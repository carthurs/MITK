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

#ifndef org_mitk_example_gui_extensionpointdefinition_Activator_H
#define org_mitk_example_gui_extensionpointdefinition_Activator_H

#include <ctkPluginActivator.h>

#include "ChangeTextRegistry.h"

class org_mitk_example_gui_extensionpointdefinition_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_example_gui_extensionpointdefinition")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext *context) override;
  void stop(ctkPluginContext *context) override;

  /**
  * Get the Registry (for extensions).
  */
  static ChangeTextRegistry *getChangeTextRegistry();
};

#endif // org_mitk_example_gui_extensionpointdefinition_Activator_H
