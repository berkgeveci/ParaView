/*=========================================================================

  Program:   ParaView
  Module:    vtkPVIceTDisplayRenderModule.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVIceTDisplayRenderModule.h"

#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkPVApplication.h"
#include "vtkPVProcessModule.h"
#include "vtkCollection.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkCallbackCommand.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeRenderManager.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVIceTDisplayRenderModule);
vtkCxxRevisionMacro(vtkPVIceTDisplayRenderModule, "1.12");



//***************************************************************************
//===========================================================================

//----------------------------------------------------------------------------
vtkPVIceTDisplayRenderModule::vtkPVIceTDisplayRenderModule()
{
  this->CompositeID.ID = 0;
  this->Composite = 0;

  this->DisplayManagerID.ID = 0;

  this->ReductionFactor = 2;
}

//----------------------------------------------------------------------------
vtkPVIceTDisplayRenderModule::~vtkPVIceTDisplayRenderModule()
{
  vtkPVApplication *pvApp = this->PVApplication;
  
  vtkPVProcessModule * pm = 0;
  if ( pvApp )
    {
    pm = pvApp->GetProcessModule();
    }

  // Tree Composite
  if (this->DisplayManagerID.ID && pvApp && pm)
    { 
    pm->DeleteStreamObject(this->DisplayManagerID);
    pm->SendStream(vtkProcessModule::RENDER_SERVER); 
    this->DisplayManagerID.ID = 0;
    }
   if (this->CompositeID.ID && pvApp && pm)
    {  
    pm->DeleteStreamObject(this->CompositeID);
    pm->SendStream(vtkProcessModule::CLIENT); 
    pm->DeleteStreamObject(this->CompositeID);
    pm->SendStream(vtkProcessModule::RENDER_SERVER_ROOT); 
    this->CompositeID.ID = 0;
    this->Composite = 0;
    }
  else if (this->Composite)
    {
    this->Composite->Delete();
    this->Composite = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkPVIceTDisplayRenderModule::SetPVApplication(vtkPVApplication *pvApp)
{
  if (this->PVApplication)
    {
    if (pvApp == NULL)
      {
      this->PVApplication->UnRegister(this);
      this->PVApplication = NULL;
      return;
      }
    vtkErrorMacro("PVApplication already set.");
    return;
    }
  if (pvApp == NULL)
    {
    return;
    }  

  vtkPVProcessModule * pm = pvApp->GetProcessModule();
  // Maybe I should not reference count this object to avoid
  // a circular reference.
  this->PVApplication = pvApp;
  this->PVApplication->Register(this);

  this->RendererID = pm->NewStreamObject("vtkIceTRenderer");
  this->RenderWindowID = pm->NewStreamObject("vtkRenderWindow");
  
  pm->SendStream(vtkProcessModule::CLIENT|vtkProcessModule::RENDER_SERVER);
  this->Renderer = 
    vtkRenderer::SafeDownCast(
      pm->GetObjectFromID(this->RendererID));
  this->RenderWindow = 
    vtkRenderWindow::SafeDownCast(
      pm->GetObjectFromID(this->RenderWindowID));
  pm->GetStream() << vtkClientServerStream::Invoke 
                  << this->RenderWindowID
                  << "FullScreenOn"
                  << vtkClientServerStream::End;
  pm->SendStream(vtkProcessModule::RENDER_SERVER);
  if (pvApp->GetUseStereoRendering())
    {
    this->RenderWindow->StereoCapableWindowOn();
    this->RenderWindow->StereoRenderOn();
    }

  pm->GetStream() << vtkClientServerStream::Invoke
                  << this->RenderWindowID << "AddRenderer" << this->RendererID
                  << vtkClientServerStream::End;
  pm->SendStream(vtkProcessModule::CLIENT|vtkProcessModule::RENDER_SERVER);

  this->Composite = NULL;
  this->DisplayManagerID = pm->NewStreamObject("vtkIceTRenderManager");
  pm->SendStream(vtkProcessModule::CLIENT|vtkProcessModule::RENDER_SERVER);
  int *tileDim = pvApp->GetTileDimensions();
  cout << "Size: " << tileDim[0] << ", " << tileDim[1] << endl;
  pm->GetStream() <<  vtkClientServerStream::Invoke
                  << this->DisplayManagerID
                  << "SetTileDimensions"
                  << tileDim[0] << tileDim[1]
                  << vtkClientServerStream::End;

  pm->GetStream() << vtkClientServerStream::Invoke
                  << this->DisplayManagerID << "SetRenderWindow"
                  << this->RenderWindowID
                  << vtkClientServerStream::End;
  pm->SendStream(vtkProcessModule::RENDER_SERVER);
  pm->GetStream() << vtkClientServerStream::Invoke
                  << pm->GetProcessModuleID()
                  << "GetController"
                  << vtkClientServerStream::End;
  pm->GetStream() << vtkClientServerStream::Invoke
                  << this->DisplayManagerID << "SetController"
                  << vtkClientServerStream::LastResult
                  << vtkClientServerStream::End; 
  pm->GetStream() << vtkClientServerStream::Invoke
                  << this->DisplayManagerID 
                  << "InitializeRMIs"
                  << vtkClientServerStream::End;
  pm->SendStream(vtkProcessModule::RENDER_SERVER);


  // **********************************************************
  this->CompositeID = pm->NewStreamObject("vtkIceTClientCompositeManager");
  pm->SendStream(vtkProcessModule::CLIENT|vtkProcessModule::RENDER_SERVER_ROOT);

  // Clean up this mess !!!!!!!!!!!!!
  // Even a cast to vtkPVClientServerModule would be better than this.
  // How can we syncronize the process modules and render modules?
  pm->GetStream() << vtkClientServerStream::Invoke << pm->GetProcessModuleID() 
                  << "GetRenderServerSocketController"
                  << vtkClientServerStream::End;
  pm->GetStream() << vtkClientServerStream::Invoke << this->CompositeID
                  << "SetClientController" << vtkClientServerStream::LastResult
                  << vtkClientServerStream::End;
  pm->GetStream() << vtkClientServerStream::Invoke << pm->GetApplicationID() << "GetClientMode"
                  << vtkClientServerStream::End;
  pm->GetStream() << vtkClientServerStream::Invoke << this->CompositeID
                  << "SetClientFlag" << vtkClientServerStream::LastResult
                  << vtkClientServerStream::End; 
  pm->GetStream() << vtkClientServerStream::Invoke << this->CompositeID
                  << "SetRenderWindow"
                  << this->RenderWindowID
                  << vtkClientServerStream::End;
  pm->GetStream() << vtkClientServerStream::Invoke << this->CompositeID
                  << "InitializeRMIs"
                  << vtkClientServerStream::End;
  pm->GetStream() << vtkClientServerStream::Invoke << this->CompositeID
                  << "UseCompositingOn"
                  << vtkClientServerStream::End;
  // copy the stream before it is sent and reset
  vtkClientServerStream copy = pm->GetStream();
  pm->SendStream(vtkProcessModule::CLIENT); // send the stream to the client
  pm->GetStream() = copy; // now copy the copy into the current stream
  pm->SendStream(vtkProcessModule::RENDER_SERVER_ROOT); // send the same stream to the server root
}

//----------------------------------------------------------------------------
void vtkPVIceTDisplayRenderModule::StillRender()
{
  // No reduction for still render.
  if (this->PVApplication && this->DisplayManagerID.ID)
    {
    vtkPVProcessModule* pm = this->PVApplication->GetProcessModule();
    pm->GetStream() << vtkClientServerStream::Invoke << this->DisplayManagerID
                    << "SetImageReductionFactor" << 1 
                    << vtkClientServerStream::End;
    pm->SendStream(vtkProcessModule::CLIENT|vtkProcessModule::RENDER_SERVER);
    }

  this->Superclass::StillRender();
}

//----------------------------------------------------------------------------
void vtkPVIceTDisplayRenderModule::InteractiveRender()
{
  if (this->PVApplication && this->DisplayManagerID.ID)
    {
    vtkPVProcessModule* pm = this->PVApplication->GetProcessModule();
    pm->GetStream() << vtkClientServerStream::Invoke << this->DisplayManagerID
                    << "SetImageReductionFactor" << 1 
                    << vtkClientServerStream::End;
    pm->SendStream(vtkProcessModule::CLIENT|vtkProcessModule::RENDER_SERVER);
    }

  this->Superclass::InteractiveRender();
}

//----------------------------------------------------------------------------
void vtkPVIceTDisplayRenderModule::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ReductionFactor: " << this->ReductionFactor << endl;
}

