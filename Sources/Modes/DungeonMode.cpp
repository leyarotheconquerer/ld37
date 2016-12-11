//
// Created by Hazen on 12/10/2016.
//

#include "DungeonMode.h"
#include "Modes/MainMenuMode.h"
#include "Subsystems/GameMode.h"
#include "Subsystems/Map.h"
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/Urho2D/Drawable2D.h>

using namespace Ld37;
using namespace Urho3D;

DungeonMode::DungeonMode(Urho3D::Context *context) :
    Mode(context),
    context_(context)
{
}

DungeonMode::~DungeonMode()
{
}

void DungeonMode::Start()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    Log* log = GetSubsystem<Log>();
    Renderer* renderer = GetSubsystem<Renderer>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Map* map = new Map(context_);
    context_->RegisterSubsystem(map);

    input_ = GetSubsystem<Input>();

    log->Write(LOG_INFO, "Starting dungeon mode");

    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();

    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetOrthographic(true);
    camera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));
    renderer->SetViewport(0, viewport);

    SpriteSheet2D* levelSheet = cache->GetResource<SpriteSheet2D>("Textures/LevelAssetsSheet.xml");
    Sprite2D* sprite = levelSheet->GetSprite("Spawner0");

    Node* spriteNode = scene_->CreateChild("TestNode");
    StaticSprite2D* staticSprite = spriteNode->CreateComponent<StaticSprite2D>();
    staticSprite->SetSprite(sprite);

    Node* mapNode = map->Generate(scene_);
    scene_->AddChild(mapNode);

    musicNode_ = new Node(context_);
    Sound* music = cache->GetResource<Sound>("Sounds/TestTheme2.ogg");
    music->SetLooped(true);
    SoundSource* musicSource = musicNode_->CreateComponent<SoundSource>();
    musicSource->Play(music);

    File file(context_, "testScene.xml", FILE_WRITE);
    if(file.IsOpen())
    {
        scene_->SaveXML(file);
        file.Close();
    }

    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(DungeonMode, HandleKeyUp));
}

void DungeonMode::Update(float timestep)
{
    const float MOVEMENT_FACTOR = 5.0f;
    Vector2 direction(0,0);
    if(input_->GetKeyDown(KEY_UP))
    {
        direction.y_ += 1;
    }
    if(input_->GetKeyDown(KEY_DOWN))
    {
        direction.y_ += -1;
    }
    if(input_->GetKeyDown(KEY_LEFT))
    {
        direction.x_ += -1;
    }
    if(input_->GetKeyDown(KEY_RIGHT))
    {
        direction.x_ += 1;
    }
    direction.Normalize();
    if (direction != Vector2(0,0))
    {
        cameraNode_->Translate2D(direction * timestep * MOVEMENT_FACTOR);
    }
}

void DungeonMode::Stop()
{
    SoundSource* musicSource = musicNode_->GetComponent<SoundSource>();
    musicSource->Stop();
}

void DungeonMode::HandleKeyUp(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    GameMode* gameMode = GetSubsystem<GameMode>();
    Engine* engine = GetSubsystem<Engine>();
    Log* log = GetSubsystem<Log>();
    log->Write(LOG_INFO, "Got a key up event");
    using namespace KeyUp;
    switch (data[P_KEY].GetInt())
    {
        case KEY_ESCAPE:
            gameMode->RegisterMode<MainMenuMode>();
            gameMode->Next<MainMenuMode>();
            break;
        case KEY_Q:
            engine->Exit();
            break;
        default:
            break;
    }
}

void DungeonMode::HandlePause(Urho3D::StringHash type, Urho3D::VariantMap &data)
{

}
