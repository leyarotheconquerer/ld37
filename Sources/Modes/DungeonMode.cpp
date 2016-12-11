//
// Created by Hazen on 12/10/2016.
//

#include "DungeonMode.h"
#include "Components/Hero.h"
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
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Urho2D/AnimationSet2D.h>
#include <Urho3D/Urho2D/AnimatedSprite2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/Urho2D/Drawable2D.h>
#include <Urho3D/DebugNew.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/UI/Button.h>

using namespace Ld37;
using namespace Urho3D;

DungeonMode::DungeonMode(Urho3D::Context *context) :
    Mode(context),
    context_(context)
{
    context->RegisterFactory<Hero>();
}

DungeonMode::~DungeonMode()
{
}

void DungeonMode::Start() {
    Graphics *graphics = GetSubsystem<Graphics>();
    Log *log = GetSubsystem<Log>();
    Renderer *renderer = GetSubsystem<Renderer>();
    ResourceCache *cache = GetSubsystem<ResourceCache>();
    UI *ui = GetSubsystem<UI>();

    Map *map = new Map(context_);
    context_->RegisterSubsystem(map);

    input_ = GetSubsystem<Input>();

    log->Write(LOG_INFO, "Starting dungeon mode");

    XMLFile *hudFile = cache->GetResource<XMLFile>("UI/DungeonUI.xml");
    uiRoot_ = ui->LoadLayout(hudFile);
    ui->GetRoot()->AddChild(uiRoot_);

    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetOrthographic(true);
    camera->SetOrthoSize((float) graphics->GetHeight() * PIXEL_SIZE * 2);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));
    renderer->SetViewport(0, viewport);

    Node *lightNode = scene_->CreateChild("Light");
    lightNode->SetDirection(Vector3(-0.1f, -0.1f, -1));
    Light *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);

    Node *mapNode = map->Generate(scene_);
    scene_->AddChild(mapNode);

    AnimationSet2D *heroAnim = cache->GetResource<AnimationSet2D>("Textures/StickDudeWiggle.scml");
    heroNode_ = scene_->CreateChild("HeroNode");
    AnimatedSprite2D *animatedSprite = heroNode_->CreateComponent<AnimatedSprite2D>();
    animatedSprite->SetAnimationSet(heroAnim);
    animatedSprite->SetLayer(21);
    animatedSprite->SetAnimation(heroAnim->GetAnimation(0));
    Hero *hero = heroNode_->CreateComponent<Hero>();
    Space *heroSpawn = map->GetHeroSpawn();
    Vector2 spawnPos = heroSpawn->items.Front().pos;
    heroNode_->SetPosition(Vector3(spawnPos.x_, spawnPos.y_, 0));

    musicNode_ = new Node(context_);
    Sound *music = cache->GetResource<Sound>("Sounds/TestTheme2.ogg");
    music->SetLooped(true);
    SoundSource* musicSource = musicNode_->CreateComponent<SoundSource>();
    musicSource->Play(music);

    File file(context_, "testScene.xml", FILE_WRITE);
    if (file.IsOpen()) {
        scene_->SaveXML(file);
        file.Close();
    }

    Button *mainMenu = (Button *) uiRoot_->GetChild(String("MainMenu"));
    heroHealth_ = (Text *) uiRoot_->GetChild(String("HeroHealthText"), true);

    SubscribeToEvent(mainMenu, E_RELEASED, URHO3D_HANDLER(DungeonMode, HandleMainMenu));

    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(DungeonMode, HandleKeyUp));

    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(DungeonMode, HandlePostRenderUpdate));
}

void DungeonMode::Update(float timestep)
{
    const float MOVEMENT_FACTOR = 10.0f;
    Vector2 direction(0,0);
    if(input_->GetKeyDown(KEY_UP) || input_->GetKeyDown(KEY_W))
    {
        direction.y_ += 1;
    }
    if(input_->GetKeyDown(KEY_DOWN) || input_->GetKeyDown(KEY_S))
    {
        direction.y_ += -1;
    }
    if(input_->GetKeyDown(KEY_LEFT) || input_->GetKeyDown(KEY_A))
    {
        direction.x_ += -1;
    }
    if(input_->GetKeyDown(KEY_RIGHT) || input_->GetKeyDown(KEY_D))
    {
        direction.x_ += 1;
    }
    direction.Normalize();
    if (direction != Vector2(0,0))
    {
        cameraNode_->Translate2D(direction * timestep * MOVEMENT_FACTOR);
    }

    Hero* hero = heroNode_->GetComponent<Hero>();
    if (hero)
    {
        heroHealth_->SetText(String().AppendWithFormat("%d", hero->GetHeath()));
    }
}

void DungeonMode::Stop()
{
    UI* ui = GetSubsystem<UI>();
    ui->GetRoot()->RemoveChild(uiRoot_, 0);

    SoundSource* musicSource = musicNode_->GetComponent<SoundSource>();
    musicSource->Stop();

    delete scene_;
    scene_ = NULL;
}

void DungeonMode::HandleKeyUp(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    GameMode* gameMode = GetSubsystem<GameMode>();
    Engine* engine = GetSubsystem<Engine>();
    Log* log = GetSubsystem<Log>();
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

void DungeonMode::HandleMainMenu(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    GetSubsystem<GameMode>()->Next<MainMenuMode>();
}

void DungeonMode::HandlePostRenderUpdate(Urho3D::StringHash type, Urho3D::VariantMap &data) {
    Map *map = GetSubsystem<Map>();
    if (!scene_) return;
    DebugRenderer *debug = scene_->GetComponent<DebugRenderer>();
    Log *log = GetSubsystem<Log>();
    if (map && map->path_.Size())
    {
        Vector3 bias(0.f, 0.05f, 0.f);
        Color pathColor(1.f, 0.f, 0.f);
        debug->AddLine(
            Vector3(37.12f, 26.88f, 1.f),
            Vector3(map->path_[0].x_, map->path_[0].z_, 1.f),
            pathColor
        );

        if (map->path_.Size() > 1)
        {
            for (int i = 0; i < map->path_.Size() - 1; ++i) {
                Vector3 test = map->path_[i];
                Vector3 test2 = map->path_[i + 1];
                debug->AddLine(
                    Vector3(map->path_[i].x_, map->path_[i].z_, 1.f),
                    Vector3(map->path_[i + 1].x_, map->path_[i + 1].z_, 1.f),
                    pathColor
                );
            }
        }
    } else {
        log->Write(LOG_INFO, "Could not find path_");
    }
}

