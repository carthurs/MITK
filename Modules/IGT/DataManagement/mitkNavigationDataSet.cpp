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

#include "mitkNavigationDataSet.h"

mitk::NavigationDataSet::NavigationDataSet( unsigned int numTools )
  : m_NavigationDataVectors(std::vector<std::vector<mitk::NavigationData::Pointer> >(numTools, std::vector<mitk::NavigationData::Pointer>()))
{

}

mitk::NavigationDataSet::~NavigationDataSet( )
{
}

bool mitk::NavigationDataSet::InsertNavigationData( unsigned int toolIndex, mitk::NavigationData::Pointer navigationData )
{
  // test if tool with given index exist
  if ( toolIndex >= m_NavigationDataVectors.size() )
  {
    MITK_WARN("NavigationDataSet") << "There is no tool with index "<<toolIndex<<".";
    return false;
  }

  // test for consistent timestamp
  if ( m_NavigationDataVectors.at(toolIndex).size() > 0 &&
       navigationData->GetIGTTimeStamp() <= (*(m_NavigationDataVectors.at(toolIndex).end()-1))->GetIGTTimeStamp())
  {
    MITK_WARN("NavigationDataSet") << "IGTTimeStamp of new NavigationData should be greater then timestamp of last NavigationData.";
    return false;
  }

  m_NavigationDataVectors.at(toolIndex).push_back(navigationData);
  return true;
}

mitk::NavigationData::Pointer mitk::NavigationDataSet::GetNavigationDataForIndex( unsigned int toolIndex, unsigned int index ) const
{
  if ( toolIndex >= m_NavigationDataVectors.size() )
  {
    MITK_WARN("NavigationDataSet") << "There is no tool with index "<<toolIndex<<".";
    return NULL;
  }

  if ( index >= m_NavigationDataVectors.at(toolIndex).size() )
  {
    MITK_WARN("NavigationDataSet") << "There is no mitk::Navigation with index "<<index<<" for tool "<<toolIndex<<".";
    return NULL;
  }

  return m_NavigationDataVectors.at(toolIndex).at(index);
}

mitk::NavigationData::Pointer mitk::NavigationDataSet::GetNavigationDataBeforeTimestamp(
    unsigned int toolIndex, mitk::NavigationData::TimeStampType timestamp
    ) const
{
  if ( toolIndex >= m_NavigationDataVectors.size() )
  {
    MITK_WARN("NavigationDataSet") << "There is no tool with index "<<toolIndex<<".";
    return NULL;
  }

  std::vector<mitk::NavigationData::Pointer>::const_iterator it;

  // iterate through all NavigationData objects of the given tool index
  // till the timestamp of the NavigationData is greater then the given timestamp
  for (it = m_NavigationDataVectors.at(toolIndex).begin();
       it != m_NavigationDataVectors.at(toolIndex).end(); ++it)
  {
    if ( (*it)->GetIGTTimeStamp() > timestamp) { break; }
  }

  // first element was greater than timestamp -> return null
  if ( it == m_NavigationDataVectors.at(toolIndex).begin() )
  {
    MITK_WARN("NavigationDataSet") << "No NavigationData was recorded before given timestamp.";
    return NULL;
  }

  // return last element smaller than the given timestamp
  return *(it-1);
}

// ---> methods necessary for BaseData
void mitk::NavigationDataSet::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::NavigationDataSet::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

bool mitk::NavigationDataSet::VerifyRequestedRegion()
{
    return true;
}

void mitk::NavigationDataSet::SetRequestedRegion(const DataObject * )
{
}
// <--- methods necessary for BaseData
