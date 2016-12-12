//
// Created by Hazen on 12/11/2016.
//

#ifndef LD37_PLAYER_H
#define LD37_PLAYER_H

#include <Urho3D/Scene/Component.h>
#include "Subsystems/Map.h"

namespace Urho3D
{
    class Node;
}

namespace Ld37
{
    enum Direction {
        CENTER = 0, NORTH = 1, EAST, SOUTH, WEST
    };
    const float MOVEMENT_TIME = 0.5f;
    const Urho3D::Vector2 PLAYER_OFFSET = {ROOM_SIZE / 2 - TILE_SIZE / 2, ROOM_SIZE / 2 - TILE_SIZE / 2};

    /// The player class, yep... that's pretty much it
    class Player : public Urho3D::Component
    {
        URHO3D_OBJECT(Player, Urho3D::Component);

    public:
        /// Constructor
        Player(Urho3D::Context* context);

        Item* GetItem() { return &item_; }

        /// Destructor
        virtual ~Player();

    protected:
        /// Handle node being assigned
        virtual void OnNodeSet(Urho3D::Node* node);

    private:
        /// Handle update events
        void HandleUpdate(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// Handle key down events
        void HandleKeyDown(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// Handle key up events
        void HandleKeyUp(Urho3D::StringHash type, Urho3D::VariantMap& data);

        Item item_;


        /// Index position
        Urho3D::IntVector2 idx_;

        /// Target movement index position
        Urho3D::IntVector2 targetIdx_;

        /// Next direction
        Direction nextDir_ = CENTER;

        /// Time remaining in current movement
        float remaining_ = 0.f;
    };
}


#endif //LD37_PLAYER_H
