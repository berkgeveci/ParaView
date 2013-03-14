/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile: vtkPVTrivialProducer.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVTrivialProducer - specialized subclass of vtkTrivialProducer that
// preserves the information about the whole extent of the data object.
// .SECTION Description
// vtkPVTrivialProducer is specialized subclass of vtkTrivialProducer that
// preserves the information about the whole extent of the data object.
// In REQUEST_INFORMATION pass, vtkTrivialProducer tells the downstream pipeline
// that the whole extent of the data is exactly equal to the extent of the data.
// That way, no filters downstream can ask for more that what's available.
// However, in some cases, we still want to provide the downstream pipeline
// information that the data is only a chunk of bigger dataset eg.
// vtkPVGeometryFilter to avoid false boundaries between structured blocks.
// Hence we use vtkPVTrivialProducer in that case. 

#ifndef __vtkPVTrivialProducer_h
#define __vtkPVTrivialProducer_h

#include "vtkTrivialProducer.h"
#include "vtkPVVTKExtensionsCoreModule.h" // needed for export macro

struct vtkPVTrivialProducerInternal;

class VTKPVVTKEXTENSIONSCORE_EXPORT vtkPVTrivialProducer : public vtkTrivialProducer
{
public:
  static vtkPVTrivialProducer* New();
  vtkTypeMacro(vtkPVTrivialProducer, vtkTrivialProducer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the output data object as well as time information
  // for the requests.
  virtual void SetOutput(vtkDataObject* output);
  virtual void SetOutput(vtkDataObject* output, double time);

  // Description:
  // If the output of the filter is topologically regular and
  // this filter is used in parallel with the grid using partitioned
  // subextents then each process will only know about its own
  // subextent.  This function does an allreduce to make sure
  // that each process knows the subextent of every process.
  void GatherExtents();

  // Description:
  // Process upstream/downstream requests trivially.  The associated
  // output data object is never modified, but it is queried to
  // fulfill requests.
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);
//BTX
protected:
  vtkPVTrivialProducer();
  ~vtkPVTrivialProducer();

  // Description:
  // Used to store any time step information. It assumes that the
  // time steps are ordered oldest to most recent.
  vtkPVTrivialProducerInternal* Internals;

private:
  vtkPVTrivialProducer(const vtkPVTrivialProducer&); // Not implemented
  void operator=(const vtkPVTrivialProducer&); // Not implemented
//ETX
};

#endif
