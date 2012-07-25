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
// .NAME vtkSMAnimationScene - extends vtkPVAnimationScene to add awareness
// about view proxies.
// .SECTION Description
// vtkSMAnimationScene subclasses vtkPVAnimationScene to add awareness about
// view proxies so that the scene can update view proxies using properties.
// vtkSMAnimationScene forwards the vtkCommand::StartEvent and
// vtkCommand::EndEvent from vtkCompositeAnimationPlayer to mark the start and
// end of animation playback.
#ifndef __vtkSMAnimationScene_h
#define __vtkSMAnimationScene_h

#include "vtkPVServerManagerDefaultModule.h" //needed for exports
#include "vtkPVAnimationScene.h"

class vtkCompositeAnimationPlayer;
class vtkEventForwarderCommand;
class vtkSMProxy;
class vtkSMViewProxy;

class VTKPVSERVERMANAGERDEFAULT_EXPORT vtkSMAnimationScene : public vtkPVAnimationScene
{
public:
  static vtkSMAnimationScene* New();
  vtkTypeMacro(vtkSMAnimationScene, vtkPVAnimationScene);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add view proxies that are involved in the animation generated by this
  // scene. When playing the animation, the scene will call StillRender() on
  // the view proxies it is aware of, also updating any caching parameters.
  void AddViewProxy(vtkSMViewProxy* proxy);
  void RemoveViewProxy(vtkSMViewProxy* proxy);
  void RemoveAllViewProxies();

  // Description;
  // Access the view proxies.
  unsigned int GetNumberOfViewProxies();
  vtkSMViewProxy* GetViewProxy(unsigned int cc);

  // Description:
  // Set if caching is enabled.
  // If Caching is true, then on every time-step, this will update the UseCache
  // and CacheKey properties on each of the views.
  vtkSetMacro(Caching, bool);
  vtkGetMacro(Caching, bool);

  // Description:
  // Set the cache limit in KBs.
  void SetCacheLimit(unsigned long kbs);

  // Description:
  // Set the time keeper. Time keeper is used to obtain the information about
  // timesteps. This is required to play animation in "Snap To Timesteps" mode.
  void SetTimeKeeper(vtkSMProxy*);
  vtkGetObjectMacro(TimeKeeper, vtkSMProxy);

  // Description:
  // Lock the start time. When locked, the StartTime won't be automatically
  // updated when data time changes.
  vtkSetMacro(LockStartTime, bool);
  vtkGetMacro(LockStartTime, bool);
  vtkBooleanMacro(LockStartTime, bool);

  // Description:
  // Lock the end time. When locked, the EndTime won't be automatically updated
  // when the data time changes.
  vtkSetMacro(LockEndTime, bool);
  vtkGetMacro(LockEndTime, bool);
  vtkBooleanMacro(LockEndTime, bool);

  // Description:
  // Forwarded to vtkCompositeAnimationPlayer.
  void SetLoop(int val);
  int GetLoop();
  void Play();
  void Stop();
  void GoToNext();
  void GoToPrevious();
  void GoToFirst();
  void GoToLast();
  void SetPlayMode(int val);
  void SetNumberOfFrames(int val);
  void SetDuration(int val);
  void SetFramesPerTimestep(int val);

//BTX
protected:
  vtkSMAnimationScene();
  ~vtkSMAnimationScene();

  // Description:
  // Overridden to ensure that caching parameters are passed to the view
  // correctly.
  virtual void TickInternal(
    double currenttime, double deltatime, double clocktime);

  // Description:
  // Called when the timekeeper's time range changes.
  void TimeKeeperTimeRangeChanged();
  void TimeKeeperTimestepsChanged();

  bool Caching;
  bool LockStartTime;
  bool LockEndTime;
  vtkSMProxy* TimeKeeper;
  vtkCompositeAnimationPlayer* AnimationPlayer;
  vtkEventForwarderCommand* Forwarder;

  friend class vtkSMAnimationSceneImageWriter;
  bool OverrideStillRender;
  vtkSetMacro(OverrideStillRender, bool);

private:
  vtkSMAnimationScene(const vtkSMAnimationScene&); // Not implemented
  void operator=(const vtkSMAnimationScene&); // Not implemented

  class vtkInternals;
  vtkInternals* Internals;
  unsigned long TimeRangeObserverID;
  unsigned long TimestepValuesObserverID;
//ETX
};

#endif
