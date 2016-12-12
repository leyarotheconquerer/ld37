//
// Created by Hazen on 12/11/2016.
//

#include "Hero.h"
#include "Subsystems/Map.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

using namespace Ld37;
using namespace Urho3D;

Hero::Hero(Urho3D::Context* context) :
    Component(context),
    health_(100),
    defense_(1),
    attack_(2)
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
    Log* log = GetSubsystem<Log>();
    Map* map = GetSubsystem<Map>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    using namespace Update;
    float timeStep = data[P_TIMESTEP].Get<float>();
    Node* heroNode = GetNode();
    SpriteSheet2D* levelSheet = cache->GetResource<SpriteSheet2D>("Textures/LevelAssetsSheet.xml");

    updateTick_ -= timeStep;

    if (currentMode_ == MOVING)
    {
        if (updateTick_ <= 0 || !currentPath_.Size())
        {
            updateTick_ = UPDATE_RATE;

            Space* exit = map->GetHeroExit();
            Space* currentRoom = map->GetSpaceWorld(heroNode->GetPosition2D());
            currentPath_ = map->GetPath(heroNode->GetPosition2D(), exit->items.Front().pos);
            if (currentRoom->items.Size())
            {
                currentItem_ = &currentRoom->items.Front();
                if (currentItem_->type != Item::HERO_SPAWNER &&
                    currentItem_->type != Item::EXIT &&
                    !currentItem_->triggered)
                {
                    currentMode_ = SEEKING;
                    updateTick_ = 0;
                }
            }
        }

        Vector2 delta = currentPath_.Front() - heroNode->GetPosition2D();

        if (delta.Length() < 0.02f)
        {
            currentPath_.Erase(0);
        }
        delta.Normalize();
        delta *= timeStep * HERO_MOVEMENT_RATE;

        heroNode->Translate2D(delta);
    }
    else if (currentMode_ == SEEKING)
    {
        if (updateTick_ <= 0)
        {
            updateTick_ = UPDATE_RATE;
            Space* currentRoom = map->GetSpaceWorld(heroNode->GetPosition2D());
            currentPath_ = map->GetPath(heroNode->GetPosition2D(), currentRoom->items.Front().pos);
            if (!currentPath_.Size())
            {
                currentMode_ = MOVING;
                updateTick_ = 0;
            }
        }
        Vector2 delta = currentPath_.Front() - heroNode->GetPosition2D();
        if (delta.Length() < 0.02f)
        {
            currentPath_.Erase(0);
            if (!currentPath_.Size())
            {
                Space* currentRoom = map->GetSpaceWorld(heroNode->GetPosition2D());
                currentItem_->triggered = true;
                if (currentItem_->type == Item::HOLE)
                {
                    log->Write(LOG_DEBUG, String("Hero found a hole in")
                               .AppendWithFormat(" (%d,%d)",
                               currentRoom->idx.x_, currentRoom->idx.y_)
                    );
                    Node* coverNode = currentItem_->node->GetChild("Cover");
                    currentItem_->node->RemoveChild(coverNode);

                    ResolveAttack(HOLE_MIN_ATTACK, HOLE_MAX_ATTACK);
                }
                else if (currentItem_->type == Item::TREASURE ||
                    currentItem_->type == Item::FALSE_TREASURE)
                {
                    StaticSprite2D* staticSprite = currentItem_->node->GetComponent<StaticSprite2D>();
                    String spriteName = staticSprite->GetSprite()->GetName();
                    spriteName.Replace("Close", "Open");
                    Sprite2D* newSprite = levelSheet->GetSprite(spriteName);
                    staticSprite->SetSprite(newSprite);

                    if (currentItem_->type == Item::TREASURE)
                    {
                        log->Write(LOG_DEBUG, String("Hero found treasure in")
                            .AppendWithFormat(" (%d,%d)",
                                              currentRoom->idx.x_, currentRoom->idx.y_)
                        );
                        ResolveHeal(TREASURE_MIN_HEAL, TREASURE_MAX_HEAL);
                        defense_++;
                        attack_++;
                    }
                    else
                    {
                        log->Write(LOG_DEBUG, String("Hero found false treasure in")
                            .AppendWithFormat(" (%d,%d)",
                                              currentRoom->idx.x_, currentRoom->idx.y_)
                        );
                        ResolveAttack(FALSE_TREASURE_MIN_ATTACK, FALSE_TREASURE_MIN_ATTACK);
                    }
                }
                currentMode_ = MOVING;
                updateTick_ = 0;
            }
        }
        delta.Normalize();
        delta *= timeStep * HERO_MOVEMENT_RATE;

        heroNode->Translate2D(delta);
    }
    else if (currentMode_ == ATTACKING)
    {

    }
    else if (currentMode_ == DYING)
    {

    }
}

void Hero::ResolveAttack(int min, int max)
{
    if (max > min)
    {
        health_ -= (Rand() % (max - min)) + min - defense_;
    }
    else
    {
        health_ -= min - defense_;
    }
}

void Hero::ResolveHeal(int minHeal, int maxHeal)
{
    if (maxHeal > minHeal)
    {
        health_ += (Rand() % (maxHeal - minHeal)) + minHeal;
    }
    else
    {
        health_ += minHeal;
    }
}
