//
// Created by Hazen on 12/11/2016.
//

#include "Hero.h"
#include "Subsystems/Map.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Node.h>

using namespace Ld37;
using namespace Urho3D;

Hero::Hero(Urho3D::Context* context) :
    Component(context)
{
}

void Hero::OnNodeSet(Urho3D::Node *node)
{
    if (node)
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Hero, HandleUpdate));
    }
}

void Hero::HandleUpdate(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    Map* map = GetSubsystem<Map>();

    using namespace Update;
    float timeStep = data[P_TIMESTEP].Get<float>();
    Node* heroNode = GetNode();

    switch (currentMode_)
    {
        case MOVING:
            break;
        case SEEKING:
            break;
        case ATTACKING:
            break;
        case DYING:
            break;
        default:
            break;
    }
}
