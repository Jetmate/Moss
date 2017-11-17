#include "main.h"

using namespace Urho3D;

URHO3D_DEFINE_APPLICATION_MAIN(Main)

void Main::Setup()
{
  // Modify engine startup parameters
  engineParameters_[EP_WINDOW_TITLE] = GetTypeName();
  engineParameters_[EP_LOG_NAME] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
  engineParameters_[EP_FULL_SCREEN] = false;
  engineParameters_[EP_HEADLESS] = false;
  engineParameters_[EP_SOUND] = false;
}

void Main::Start()
{
  // Create the scene content
  CreateScene();

  // Setup the viewport for displaying the scene
  SetupViewport();

  // Hook up to the frame update events
  SubscribeToEvents();
}

void Main::Stop()
{
  engine_->DumpResources(true);
}

void Main::CreateScene()
{
  ResourceCache *cache = GetSubsystem<ResourceCache>();

  scene_ = new Scene(context_);


  // Create the Octree component to the scene so that drawable objects can be rendered. Use default volume
  // (-1000, -1000, -1000) to (1000, 1000, 1000)
  scene_->CreateComponent<Octree>();

  // Create a Zone component into a child scene node. The Zone controls ambient lighting and fog settings. Like the Octree,
  // it also defines its volume with a bounding box, but can be rotated (so it does not need to be aligned to the world X, Y
  // and Z axes.) Drawable objects "pick up" the zone they belong to and use it when rendering; several zones can exist
  Node *zoneNode = scene_->CreateChild("Zone");
  Zone *zone = zoneNode->CreateComponent<Zone>();
  // Set same volume as the Octree, set a close bluish fog and some ambient light
  zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
  zone->SetAmbientColor(Color(0.05f, 0.1f, 0.15f));
  zone->SetFogColor(Color(0.1f, 0.2f, 0.3f));
  zone->SetFogStart(10.0f);
  zone->SetFogEnd(100.0f);

  // Create box StaticModels in the scene
  const int NUM_OBJECTS = 100;
  for (unsigned i = 0; i < NUM_OBJECTS; ++i)
  {
    Node *boxNode = scene_->CreateChild("Box");
    boxNode->SetPosition(Vector3(0, 0, 10 * i));

    StaticModel *boxObject = boxNode->CreateComponent<StaticModel>();
    boxObject->SetModel(cache->GetResource<Model>("Models/cube.ply.mdl"));
    // boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
  }

  // Create the camera. Let the starting position be at the world origin. As the fog limits maximum visible distance, we can
  // bring the far clip plane closer for more effective culling of distant objects
  cameraNode_ = scene_->CreateChild("Camera");
  Camera *camera = cameraNode_->CreateComponent<Camera>();
  camera->SetFarClip(100.0f);

  // Create a point light to the camera scene node
  Light *light = cameraNode_->CreateComponent<Light>();
  light->SetLightType(LIGHT_POINT);
  light->SetRange(30.0f);
}

void Main::SetupViewport()
{
  Renderer *renderer = GetSubsystem<Renderer>();

  // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
  SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
  renderer->SetViewport(0, viewport);
}

void Main::SubscribeToEvents()
{
  // Subscribe HandleUpdate() function for processing update events
  SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
}

void Main::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
  using namespace Update;

  // Take the frame time step, which is stored as a float
  float timeStep = eventData[P_TIMESTEP].GetFloat();

  // Move the camera, scale movement with time step
  MoveCamera(timeStep);
}

void Main::MoveCamera(float timeStep)
{
  Input *input = GetSubsystem<Input>();

  // Movement speed as world units per second
  const float MOVE_SPEED = 20.0f;
  // Mouse sensitivity as degrees per pixel
  const float MOUSE_SENSITIVITY = 0.1f;

  // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
  IntVector2 mouseMove = input->GetMouseMove();
  yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
  pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
  pitch_ = Clamp(pitch_, -90.0f, 90.0f);

  // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
  cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

  // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
  if (input->GetKeyDown(KEY_W))
    cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
  if (input->GetKeyDown(KEY_S))
    cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
  if (input->GetKeyDown(KEY_A))
    cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
  if (input->GetKeyDown(KEY_D))
    cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
}
