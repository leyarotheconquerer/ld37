//
// Created by Hazen on 12/10/2016.
//

#include "Ld37.h"
#include "Subsystems/GameMode.h"
#include "Modes/MainMenuMode.h"
#include "Modes/DungeonMode.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>

using namespace Ld37;
using namespace Urho3D;

URHO3D_DEFINE_APPLICATION_MAIN(Ld37App);

Ld37App::Ld37App(Context* context) :
    Application(context),
    context_(context)
{
}

void Ld37App::Setup()
{
    engineParameters_["LogLevel"] = LOG_INFO;
    engineParameters_["WindowTitle"] = "Ludum Dare 37";
    engineParameters_["FullScreen"] = false;
    engineParameters_["Sound"] = true;
    engineParameters_["EventProfiler"] = true;
}

void Ld37App::Start()
{
    // Load relevant subsystems
    Graphics* graphics = GetSubsystem<Graphics>();
    Input* input = GetSubsystem<Input>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Renderer* renderer = GetSubsystem<Renderer>();
    UI* ui = GetSubsystem<UI>();

    // Create and register the game mode subsystem
    GameMode* gameMode = new GameMode(context_);
    context_->RegisterSubsystem(gameMode);

    // Load the default UI styles
    XMLFile* style = cache->GetResource<XMLFile>("UI/Ld37Style.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    // Make the cursor visible and tell it to use the current style
    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto();
    ui->SetCursor(cursor);
    input->SetMouseVisible(true);
    input->SetMousePosition(IntVector2(
        graphics->GetWidth() / 2,
        graphics->GetHeight() / 2
    ));

    // Set the background to a nice grey
    Zone* zone = renderer->GetDefaultZone();
    zone->SetFogColor(Color(.36f, .38f, .47f, 1.f));

    // Load the initial game mode
    gameMode->Initialize<DungeonMode>();
}

void Ld37App::Stop()
{

}
