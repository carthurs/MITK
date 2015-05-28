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

#include "mitkBaseDataSerializer.h"
#include "mitkStandardFileLocations.h"
#include <itksys/SystemTools.hxx>

#include <Poco/TemporaryFile.h>
#include <Poco/Path.h>

mitk::BaseDataSerializer::BaseDataSerializer()
: m_FilenameHint("unnamed")
, m_WorkingDirectory("")
{
}

mitk::BaseDataSerializer::~BaseDataSerializer()
{
}

std::string mitk::BaseDataSerializer::Serialize()
{
  MITK_INFO << this->GetNameOfClass()
           << " is asked to serialize an object " << (const void*) this->m_Data
           << " into a directory " << m_WorkingDirectory
           << " using a filename hint " << m_FilenameHint;

  return "";
}

std::string mitk::BaseDataSerializer::GetUniqueFilenameInWorkingDirectory()
{
    return Poco::Path(Poco::TemporaryFile::tempName(m_WorkingDirectory)).getFileName();
}

