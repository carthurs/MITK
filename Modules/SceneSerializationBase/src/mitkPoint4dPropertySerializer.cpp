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

#ifndef mitkPoint4dPropertySerializer_h_included
#define mitkPoint4dPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include "mitkStringsToNumbers.h"
#include <mitkLocaleSwitch.h>

namespace mitk
{
  class Point4dPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(Point4dPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      virtual TiXmlElement *Serialize() override
    {
      if (const Point4dProperty *prop = dynamic_cast<const Point4dProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto element = new TiXmlElement("point");
        Point4D point = prop->GetValue();
        element->SetAttribute("x", boost::lexical_cast<std::string>(point[0]));
        element->SetAttribute("y", boost::lexical_cast<std::string>(point[1]));
        element->SetAttribute("z", boost::lexical_cast<std::string>(point[2]));
        element->SetAttribute("t", boost::lexical_cast<std::string>(point[3]));
        return element;
      }
      else
        return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement *element) override
    {
      if (!element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      std::string v_str[4];
      if (element->QueryStringAttribute("x", &v_str[0]) != TIXML_SUCCESS)
        return nullptr;
      if (element->QueryStringAttribute("y", &v_str[1]) != TIXML_SUCCESS)
        return nullptr;
      if (element->QueryStringAttribute("z", &v_str[2]) != TIXML_SUCCESS)
        return nullptr;
      if (element->QueryStringAttribute("t", &v_str[3]) != TIXML_SUCCESS)
        return nullptr;
      Point4D v;
      try
      {
        StringsToNumbers<double>(4, v_str, v);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }
      return Point4dProperty::New(v).GetPointer();
    }

  protected:
    Point4dPropertySerializer() {}
    virtual ~Point4dPropertySerializer() {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point4dPropertySerializer);

#endif
