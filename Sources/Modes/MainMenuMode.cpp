//
// Created by Hazen on 12/10/2016.
//

#include "MainMenuMode.h"
#include <Urho3D/IO/Log.h>

using namespace Ld37;
using namespace Urho3D;

MainMenuMode::MainMenuMode(Urho3D::Context* context) :
    Mode(context),
    context_(context)
{
}

void MainMenuMode::Start()
{
    Log* log = GetSubsystem<Log>();
    log->Write(LOG_INFO, "Starting Main Menu Mode");
}

void MainMenuMode::Update(float timestep)
{

}

void MainMenuMode::Stop()
{

}
