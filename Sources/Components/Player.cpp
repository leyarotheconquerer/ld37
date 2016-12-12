//
// Created by Hazen on 12/11/2016.
//

#include "Player.h"
#include "Subsystems/Map.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Node.h>

using namespace Ld37;
using namespace Urho3D;

Player::Player(Urho3D::Context *context) :
    Component(context)
{
}

Player::~Player()
{
}

void Player::OnNodeSet(Urho3D::Node *node)
{
    if (node)
    {
        Map* map = GetSubsystem<Map>();
        Space* spawnSpace = map->GetPlayerSpawn();
        idx_ = spawnSpace->idx;
        targetIdx_ = spawnSpace->idx;

        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Player, HandleUpdate));
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Player, HandleKeyDown));
        SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Player, HandleKeyUp));
    }
}

void Player::HandleUpdate(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    Map* map = GetSubsystem<Map>();
    using namespace Update;
    float timestep = data[P_TIMESTEP].GetFloat();
    remaining_ -=  timestep;
}

void Player::HandleKeyDown(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    Log* log = GetSubsystem<Log>();
    Map* map = GetSubsystem<Map>();

    if (remaining_ <= 0.f)
    {
        Space* currentRoom = map->GetSpaceWorld(GetNode()->GetPosition2D());
        Space* nextRoom;
        using namespace KeyDown;
        switch (data[P_KEY].GetInt())
        {
            case KEY_W:
            case KEY_UP:
                nextRoom = map->GetSpaceIndex(currentRoom->idx + IntVector2(0,1));
                nextDir_ = NORTH;
                break;
            case KEY_D:
            case KEY_RIGHT:
                nextRoom = map->GetSpaceIndex(currentRoom->idx + IntVector2(1,0));
                nextDir_ = EAST;
                break;
            case KEY_S:
            case KEY_DOWN:
                nextRoom = map->GetSpaceIndex(currentRoom->idx + IntVector2(0,-1));
                nextDir_ = SOUTH;
                break;
            case KEY_A:
            case KEY_LEFT:
                nextRoom = map->GetSpaceIndex(currentRoom->idx + IntVector2(-1,0));
                nextDir_ = WEST;
                break;
            default:
                break;
        }

        if (nextRoom)
        {
            GetNode()->SetPosition(nextRoom->pos + PLAYER_OFFSET);
            remaining_ = MOVEMENT_TIME;
        }
    }
}

void Player::HandleKeyUp(Urho3D::StringHash type, Urho3D::VariantMap &data)
{
    Map* map = GetSubsystem<Map>();

    Space* currentRoom = map->GetSpaceWorld(GetNode()->GetPosition2D());

    using namespace KeyUp;
    switch(data[P_KEY].GetInt())
    {
        case KEY_SPACE:
            if(currentRoom->items.Size())
            {
                item_ = currentRoom->items.Front();
                currentRoom->items.Erase(0);
            }
            break;
        default:
            break;
    }
}
