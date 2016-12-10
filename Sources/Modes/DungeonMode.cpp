//
// Created by Hazen on 12/10/2016.
//

#include "DungeonMode.h"
#include <Urho3D/IO/Log.h>

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
    Log* log = GetSubsystem<Log>();
    log->Write(LOG_INFO, "Starting dungeon mode");
}

void DungeonMode::Update(float timestep)
{

}

void DungeonMode::Stop()
{

}

void DungeonMode::HandlePause(Urho3D::StringHash type, Urho3D::VariantMap &data)
{

}
