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

  // Hook up to the frame update events  SubscribeToEvents();
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

  Node *lightNode = scene_->CreateChild("DirectionalLight");
  lightNode->SetDirection(Vector3(0, -1, 0)); // The direction vector does not need to be normalized
  lightNode->SetPosition(Vector3(0, 100, 0));
  Light *light = lightNode->CreateComponent<Light>();
  light->SetLightType(LIGHT_DIRECTIONAL);
  light->SetRange(1000.0f);

  // Create a Zone component into a child scene node. The Zone controls ambient lighting and fog settings. Like the Octree,
  // it also defines its volume with a bounding box, but can be rotated (so it does not need to be aligned to the world X, Y
  // and Z axes.) Drawable objects "pick up" the zone they belong to and use it when rendering; several zones can exist
  Node *zoneNode = scene_->CreateChild("Zone");
  Zone *zone = zoneNode->CreateComponent<Zone>();
  // Set same volume as the Octree, set a close bluish fog and some ambient light
  zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
  zone->SetAmbientColor(Color(1, 1, 1));
  zone->SetFogColor(Color(0.1f, 0.2f, 0.3f));
  zone->SetFogStart(10.0f);
  zone->SetFogEnd(100.0f);

  // zone->SetLightMask(100)

  // Create box StaticModels in the scene
  // String types[] = {"cube.ply.mdl", "cube.obj.mdl", "cube.mc.ply.mdl"};
  // for (int i = 0; i < sizeof(types) / sizeof(types[0]); i++)
  // {
  //   Node *boxNode = scene_->CreateChild("Box");
  //   boxNode->SetPosition(Vector3(0, 0, 20 * i));

  //   StaticModel *boxObject = boxNode->CreateComponent<StaticModel>();
  //   boxObject->SetModel(cache->GetResource<Model>("Models/" + types[i]));
  //   // boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
  // }

  JSONFile *jf = cache->GetResource<JSONFile>("Maps/file.json");
  JSONValue &root = jf->GetRoot();
  JSONObject json = root.GetObject();
  JSONArray blocks = json["blocks"].GetArray();

  for (RandomAccessIterator<JSONValue> it = blocks.Begin(); it != blocks.End(); ++it) {
    JSONObject object = it->GetObject();

    Node *boxNode = scene_->CreateChild("Box");
    boxNode->SetPosition(Vector3(object["x"].GetInt() * 10, object["y"].GetInt() * 10, object["z"].GetInt() * 10));
    StaticModel *boxObject = boxNode->CreateComponent<StaticModel>();
    boxObject->SetModel(cache->GetResource<Model>("Models/cube.obj.mdl"));
    boxObject->SetMaterial(cache->GetResource<Material>("Materials/cube.xml"));
  }

  // for (RandomAccessIterator<Variant> it = blocks.Begin(); it != blocks.End(); ++it)
  // {
  //   URHO3D_LOGINFO(it->GetVariantMap()["x"]->GetString());
  // }

  // VariantMap json;
  // for (JSONObjectIterator it = root.Begin(); it != root.End(); ++it)
  // {
  //   URHO3D_LOGINFO((String) it->second_.GetArray()[0].GetObject()["x"]->GetInt());
  //   // for (JSONObjectIterator it2 = it->second_.Begin(); it2 != it->second_.End(); ++it2)
  //   // {
  //   //   URHO3D_LOGINFO((String) it2->second_.GetVariantMap().Keys().Size());
  //   // }
  //   // VariantVector vector = it->second_.GetVariantVector();
  //   // URHO3D_LOGINFO((String) vector.Size());

  //   // for (RandomAccessIterator<Variant> it2 = vector.Begin(); it2 < vector.End(); ++it2)
  //   // {
  //   //   // *it2 = it2->GetVariantMap();
  //   //   URHO3D_LOGINFO((String) (*it2).GetVariantMapPtr()->Keys().Size());
  //   // }
  //   // json[it->first_] = vector;
  // }

  // Node *boxNode = scene_->CreateChild("Box");
  // boxNode->SetPosition(Vector3(0, -10, 20));
  // StaticModel *boxObject = boxNode->CreateComponent<StaticModel>();
  // boxObject->SetModel(cache->GetResource<Model>("Models/cube.obj.mdl"));
  // boxObject->SetMaterial(cache->GetResource<Material>("Materials/cube.xml"));

  // boxNode = scene_->CreateChild("Box");
  // boxNode->SetPosition(Vector3(0, -10, 40));
  // boxObject = boxNode->CreateComponent<StaticModel>();
  // boxObject->SetModel(cache->GetResource<Model>("Models/cube.obj.mdl"));

  // Create the camera. Let the starting position be at the world origin. As the fog limits maximum visible distance, we can
  // bring the far clip plane closer for more effective culling of distant objects
  cameraNode_ = scene_->CreateChild("Camera");
  Camera *camera = cameraNode_->CreateComponent<Camera>();
  // camera->SetFarClip(50.0f);

  // Create a point light to the camera scene node
  // Light* light = cameraNode_->CreateComponent<Light>();
  // light->SetLightType(LIGHT_SPOT);
  // light->SetRange(30.0f);
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
