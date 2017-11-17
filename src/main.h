#pragma once

#include <memory>
#include "includes.h"

class Main : public Urho3D::Application
{
  URHO3D_OBJECT(Main, Application);

public:
  Main(Urho3D::Context *context) : Application(context) {};

  /// Setup before engine initialization. Modifies the engine parameters.
  virtual void Setup();
  /// Setup after engine initialization. Creates the logo, console & debug HUD.
  virtual void Start();
  /// Cleanup after the main loop. Called by Application.
  virtual void Stop();

private:
  /// Construct the scene content.
  void CreateScene();
  /// Set up a viewport for displaying the scene.
  void SetupViewport();
  /// Subscribe to application-wide logic update events.
  void SubscribeToEvents();
  /// Read input and moves the camera.
  void MoveCamera(float timeStep);
  // / Handle the logic update event.
  void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

  /// Scene.
  Urho3D::SharedPtr<Urho3D::Scene> scene_;
  /// Camera scene node.
  Urho3D::SharedPtr<Urho3D::Node> cameraNode_;
  /// Camera yaw angle.
  float yaw_;
  /// Camera pitch angle.
  float pitch_;
};
