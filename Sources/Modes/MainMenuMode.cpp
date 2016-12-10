//
// Created by Hazen on 12/10/2016.
//

#include "MainMenuMode.h"
#include "DungeonMode.h"
#include "Subsystems/GameMode.h"
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>

using namespace Ld37;
using namespace Urho3D;

MainMenuMode::MainMenuMode(Urho3D::Context* context) :
    Mode(context),
    context_(context)
{
}

MainMenuMode::~MainMenuMode()
{
    // This may be dangerous...
    delete scene_;
}

void MainMenuMode::Start()
{
    Input* input = GetSubsystem<Input>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Log* log = GetSubsystem<Log>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    log->Write(LOG_INFO, "Starting Main Menu Mode");

    XMLFile* style = cache->GetResource<XMLFile>("UI/Ld37Style.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto();
    ui->SetCursor(cursor);
    input->SetMouseVisible(true);
    input->SetMousePosition(IntVector2(
        graphics->GetWidth() / 2,
        graphics->GetHeight() / 2
    ));

    XMLFile* mainMenu = cache->GetResource<XMLFile>("UI/MainMenu.xml");
    uiRoot_ = ui->LoadLayout(mainMenu);
    ui->GetRoot()->AddChild(uiRoot_);

    scene_ = new Scene(context_);

    Sound* music = cache->GetResource<Sound>("Sounds/TestTheme.ogg");
    if (music)
    {
        music->SetLooped(true);
        musicNode_ = scene_->CreateChild("Music");
        SoundSource* soundSource = musicNode_->CreateComponent<SoundSource>();
        soundSource->Play(music);
        soundSource->SetGain(0.75f);
    }

    Button* play = (Button*)uiRoot_->GetChild(String("Play"));
    Button* exit = (Button*)uiRoot_->GetChild(String("Exit"));

    SubscribeToEvent(play, E_RELEASED, URHO3D_HANDLER(MainMenuMode, HandlePlay));
    SubscribeToEvent(exit, E_RELEASED, URHO3D_HANDLER(MainMenuMode, HandleExit));
}

void MainMenuMode::Update(float timestep)
{

}

void MainMenuMode::Stop()
{
    Log* log = GetSubsystem<Log>();

    log->Write(LOG_INFO, "Stopping main menu");

    // Stop the music
    SoundSource* soundSource = musicNode_->GetComponent<SoundSource>();
    soundSource->Stop();

    // Remove the root element from the UI tree
    UIElement* root = uiRoot_->GetParent();
    root->RemoveChild(uiRoot_);
}

void MainMenuMode::HandlePlay(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    GameMode* gameMode = GetSubsystem<GameMode>();
    gameMode->Next<DungeonMode>();
}

void MainMenuMode::HandleExit(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    Engine* engine = GetSubsystem<Engine>();
    engine->Exit();
}
