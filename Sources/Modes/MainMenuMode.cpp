//
// Created by Hazen on 12/10/2016.
//

#include "MainMenuMode.h"
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
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
    Log* log = GetSubsystem<Log>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();
    Input* input = GetSubsystem<Input>();

    XMLFile* style = cache->GetResource<XMLFile>("UI/Ld37Style.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto();
    ui->SetCursor(cursor);
    input->SetMouseVisible(true);

    XMLFile* mainMenu = cache->GetResource<XMLFile>("UI/MainMenu.xml");
    uiRoot_ = ui->LoadLayout(mainMenu);
    ui->GetRoot()->AddChild(uiRoot_);

    log->Write(LOG_INFO, "Starting Main Menu Mode");

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

}

void MainMenuMode::Update(float timestep)
{

}

void MainMenuMode::Stop()
{

}
