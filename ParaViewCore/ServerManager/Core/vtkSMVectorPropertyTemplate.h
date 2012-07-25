/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMVectorPropertyTemplate
// .SECTION Description
//

#ifndef __vtkSMVectorPropertyTemplate_h
#define __vtkSMVectorPropertyTemplate_h

#include <assert.h>
#include <vector>
#include <algorithm>
#include <string>
#include <vtksys/ios/sstream>
#include "vtkCommand.h"
#include <vtkPVXMLElement.h>
#include "vtkSMProperty.h"
#include <typeinfo>
#include <limits>

class vtkSMProperty;

template <class T>
class vtkSMVectorPropertyTemplate
{
  vtkSMProperty* Property;
public:
  std::vector<T> Values;
  std::vector<T> UncheckedValues;
  std::vector<T> DefaultValues; // Values set in the XML configuration.
  bool DefaultsValid;
  bool Initialized;

  //---------------------------------------------------------------------------
  vtkSMVectorPropertyTemplate(vtkSMProperty* property)
    {
    this->Property = property;
    this->DefaultsValid = false;
    this->Initialized = false;
    }

  //---------------------------------------------------------------------------
  void UpdateDefaultValues()
    {
    this->DefaultValues.clear();
    this->DefaultValues.insert(this->DefaultValues.end(),
      this->Values.begin(), this->Values.end());
    this->DefaultsValid = true;
    }

  //---------------------------------------------------------------------------
  void SetNumberOfUncheckedElements(unsigned int num)
    {
    this->UncheckedValues.resize(num);
    this->Property->InvokeEvent(vtkCommand::UncheckedPropertyModifiedEvent);
    }

  //---------------------------------------------------------------------------
  unsigned int GetNumberOfUncheckedElements()
    {
    return static_cast<unsigned int>(this->UncheckedValues.size());
    }

  //---------------------------------------------------------------------------
  unsigned int GetNumberOfElements()
    {
    return static_cast<unsigned int>(this->Values.size());
    }

  //---------------------------------------------------------------------------
  void SetNumberOfElements(unsigned int num)
    {
    if (num == this->Values.size())
      {
      return;
      }
    this->Values.resize(num);
    this->UncheckedValues.resize(num);
    if (num == 0)
      {
      // If num == 0, then we already have the intialized values (so to speak).
      this->Initialized = true;
      }
    else
      {
      this->Initialized = false;
      }
    this->Property->Modified();
    }

  //---------------------------------------------------------------------------
  T& GetElement(unsigned int idx)
    {
    assert(idx < this->Values.size());
    return this->Values[idx];
    }

  //---------------------------------------------------------------------------
  // seems weird that this idx is "int".
  T& GetDefaultValue(int idx)
    {
    if (idx >= 0 && idx < static_cast<int>(this->DefaultValues.size()))
      {
      return this->DefaultValues[idx];
      }

    static T empty_value = T();
    return empty_value;
    }

  //---------------------------------------------------------------------------
  T* GetElements()
    {
    return (this->Values.size() > 0)?  &this->Values[0] : NULL;
    }

  //---------------------------------------------------------------------------
  T& GetUncheckedElement(unsigned int idx)
    {
    assert(idx < this->UncheckedValues.size());
    return this->UncheckedValues[idx];
    }

  //---------------------------------------------------------------------------
  void SetUncheckedElement(unsigned int idx, T value)
    {
    if (idx >= this->GetNumberOfUncheckedElements())
      {
      this->UncheckedValues.resize(idx+1);
      }

    if(this->UncheckedValues[idx] != value)
      {
      this->UncheckedValues[idx] = value;
      this->Property->InvokeEvent(vtkCommand::UncheckedPropertyModifiedEvent);
      }
    }

  //---------------------------------------------------------------------------
  int SetUncheckedElements(const T* values)
    {
    return this->SetUncheckedElements(values, this->GetNumberOfUncheckedElements());
    }

  //---------------------------------------------------------------------------
  int SetUncheckedElements(const T* values, unsigned int numValues)
    {
    bool modified = false;
    unsigned int numArgs = this->GetNumberOfUncheckedElements();
    if(numArgs != numValues)
      {
      this->UncheckedValues.resize(numValues);
      numArgs = numValues;
      modified = true;
      }
    else
      {
      modified = !std::equal(this->UncheckedValues.begin(), this->UncheckedValues.end(), values);
      }

    if(!modified)
      {
      return 1;
      }

    std::copy(values, values + numArgs, this->UncheckedValues.begin());

    this->Property->InvokeEvent(vtkCommand::UncheckedPropertyModifiedEvent);
    return 1;
    }

  //---------------------------------------------------------------------------
  int SetElementAsString(unsigned int idx, const char* value)
    {
    if(!value) { return 0; }

    // Convert String to T
    T realValue;
    vtksys_ios::stringstream vstr;
    vstr << value << ends;
    vstr >> realValue;

    // SetElement
    return this->SetElement(idx, realValue);
    }
  //---------------------------------------------------------------------------
  int SetElement(unsigned int idx, T value)
    {
    unsigned int numElems = this->GetNumberOfElements();

    if (this->Initialized && idx < numElems && value == this->GetElement(idx))
      {
      return 1;
      }

    if (idx >= numElems)
      {
      this->SetNumberOfElements(idx+1);
      }
    this->Values[idx] = value;

    // Make sure to initialize BEFORE Modified() is called. Otherwise,
    // the value would not be pushed.
    this->Initialized = true;
    this->Property->Modified();

    this->ClearUncheckedElements();
    return 1;
    }

  //---------------------------------------------------------------------------
  int SetElements(const T* values)
    {
    return this->SetElements(values, this->GetNumberOfElements());
    }

  //---------------------------------------------------------------------------
  int SetElements(const T* values, unsigned int numValues)
    {
    bool modified = false;
    unsigned int numArgs = this->GetNumberOfElements();
    if (numArgs != numValues)
      {
      this->Values.resize(numValues);
      this->UncheckedValues.resize(numValues);
      numArgs = numValues;
      modified = true;
      }
    else
      {
      modified = !std::equal(this->Values.begin(), this->Values.end(), values);
      }
    if (!modified && this->Initialized)
      {
      return 1;
      }

    std::copy(values, values+numArgs, this->Values.begin());

    this->Initialized = true;
    this->Property->Modified();

    this->ClearUncheckedElements();
    return 1;
    }


  //---------------------------------------------------------------------------
  void Copy(vtkSMVectorPropertyTemplate<T>* dsrc)
    {
    if (dsrc && dsrc->Initialized)
      {
      bool modified = false;

      if (this->Values != dsrc->Values)
        {
        this->Values = dsrc->Values;
        modified = true;
        }
      // If we were not initialized, we are now modified even if the value
      // did not change
      modified = modified || !this->Initialized;
      this->Initialized = true;

      if (modified)
        {
        this->Property->Modified();
        this->ClearUncheckedElements();
        }
      }
    }

  //---------------------------------------------------------------------------
  void ResetToDefaultInternal()
    {
    if (this->DefaultsValid && this->DefaultValues != this->Values)
      {
      this->Values = this->DefaultValues;
      // Make sure to initialize BEFORE Modified() is called. Otherwise,
      // the value would not be pushed.
      this->Initialized = true;
      this->Property->Modified();
      this->ClearUncheckedElements();
      }
    }
  //---------------------------------------------------------------------------
  void SaveStateValues(vtkPVXMLElement* propertyElement)
    {
    unsigned int size = this->GetNumberOfElements();
    if (size > 0)
      {
      propertyElement->AddAttribute("number_of_elements", size);
      }
    for (unsigned int i=0; i<size; i++)
      {
      vtksys_ios::ostringstream valueAsString;

      // set the stream precision to the maximum precision for the data type
      valueAsString.precision(std::numeric_limits<T>::digits10);

      valueAsString << this->GetElement(i);

      vtkPVXMLElement* elementElement = vtkPVXMLElement::New();
      elementElement->SetName("Element");
      elementElement->AddAttribute("index", i);
      elementElement->AddAttribute("value", valueAsString.str().c_str());
      propertyElement->AddNestedElement(elementElement);
      elementElement->Delete();
      }
    }

  void ClearUncheckedElements()
    {
    // copy values to unchecked values
    this->UncheckedValues = this->Values;
    this->Property->InvokeEvent(vtkCommand::UncheckedPropertyModifiedEvent);
    }
};
#endif

// VTK-HeaderTest-Exclude: vtkSMVectorPropertyTemplate.h
