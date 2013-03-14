/*=========================================================================

  Program:   ParaView
  Module:    vtkStructuredGridVolumeRepresentation.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStructuredGridVolumeRepresentation.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCommunicator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"
#include "vtkPVRenderView.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"


vtkStandardNewMacro(vtkStructuredGridVolumeRepresentation);
//----------------------------------------------------------------------------
vtkStructuredGridVolumeRepresentation::vtkStructuredGridVolumeRepresentation()
{
  this->UseDataParititions = false;
}

//----------------------------------------------------------------------------
vtkStructuredGridVolumeRepresentation::~vtkStructuredGridVolumeRepresentation()
{
}

//----------------------------------------------------------------------------
void vtkStructuredGridVolumeRepresentation::SetUseDataParititions(bool val)
{
  if (this->UseDataParititions != val)
    {
    this->UseDataParititions = val;
    this->MarkModified();
    }
}

//----------------------------------------------------------------------------
int vtkStructuredGridVolumeRepresentation::FillInputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkStructuredGrid");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkStructuredGridVolumeRepresentation::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  if (!this->Superclass::RequestData(request, inputVector, outputVector))
    {
    return 0;
    }

  if (inputVector[0]->GetNumberOfInformationObjects()==1 &&
    this->UseDataParititions)
    {
    // reduce bounds across processes in parallel.
    vtkMultiProcessController* controller =
      vtkMultiProcessController::GetGlobalController();
    int numProcs = controller->GetNumberOfProcesses();
    if (numProcs > 1)
      {
      double bounds_max[3] = {VTK_DOUBLE_MIN, VTK_DOUBLE_MIN};
      double bounds_min[3] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MAX};
      if (vtkMath::AreBoundsInitialized(this->DataBounds))
        {
        bounds_min[0] = this->DataBounds[0];
        bounds_min[1] = this->DataBounds[2];
        bounds_min[2] = this->DataBounds[4];
        bounds_max[0] = this->DataBounds[1];
        bounds_max[1] = this->DataBounds[3];
        bounds_max[2] = this->DataBounds[5];
        }

      double reduced_bounds_max[3], reduced_bounds_min[3];
      controller->AllReduce(bounds_max, reduced_bounds_max, 3,
        vtkCommunicator::MAX_OP);
      controller->AllReduce(bounds_min, reduced_bounds_min, 3,
        vtkCommunicator::MIN_OP);

      this->DataBounds[0] = reduced_bounds_min[0];
      this->DataBounds[2] = reduced_bounds_min[1];
      this->DataBounds[4] = reduced_bounds_min[2];
      this->DataBounds[1] = reduced_bounds_max[0];
      this->DataBounds[3] = reduced_bounds_max[1];
      this->DataBounds[5] = reduced_bounds_max[2];
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkStructuredGridVolumeRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
    {
    return 0;
    }

  if (request_type == vtkPVView::REQUEST_UPDATE())
    {
    if (this->GetNumberOfInputConnections(0) == 1 &&
        this->UseDataParititions)
      {
      vtkAlgorithmOutput* connection = this->GetInputConnection(0, 0);
      vtkAlgorithm* inputAlgo = connection->GetProducer();
      vtkStreamingDemandDrivenPipeline* sddp =
        vtkStreamingDemandDrivenPipeline::SafeDownCast(inputAlgo->GetExecutive());
      vtkExtentTranslator* translator = 0;
      // TODO (berk)
      // Need to fix this
      /*
        sddp->GetExtentTranslator(connection->GetIndex());
      */

      int whole_extent[6] = {1, -1, 1, -1, 1, -1};
      sddp->GetWholeExtent(sddp->GetOutputInformation(connection->GetIndex()),
        whole_extent);

      double origin[3] = { this->DataBounds[0], this->DataBounds[2],
        this->DataBounds[4] };

      double spacing[3] = {
        (this->DataBounds[1] - this->DataBounds[0]) / (whole_extent[1] - whole_extent[0] + 1),
        (this->DataBounds[3] - this->DataBounds[2]) / (whole_extent[3] - whole_extent[2] + 1),
        (this->DataBounds[5] - this->DataBounds[4]) / (whole_extent[5] - whole_extent[4] + 1)
      };

      vtkPVRenderView::SetOrderedCompositingInformation(inInfo, this,
        translator, whole_extent, origin, spacing);
      }
    else if (!this->UseDataParititions)
      {
      double origin[3] = {0, 0, 0};
      double spacing[3] = {1, 1, 1};
      int whole_extent[6] = {1, -1, 1, -1, 1, -1};

      // Unset the ordered compositing info, so that vtkPVRenderView will
      // redistribute the unstructured grid as needed to volume render it.
      vtkPVRenderView::SetOrderedCompositingInformation(inInfo, this,
        NULL, whole_extent, origin, spacing);
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkStructuredGridVolumeRepresentation::PrintSelf(
  ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "UseDataParititions: " << this->UseDataParititions << endl;
}
