//
// Created by Hazen on 12/10/2016.
//

#include "Ld37.h"
#include "Subsystems/GameMode.h"
#include "Modes/MainMenuMode.h"

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
    GameMode* gameMode = new GameMode(context_);
    context_->RegisterSubsystem(gameMode);

    gameMode->Initialize<MainMenuMode>();
}

void Ld37App::Stop()
{

}
