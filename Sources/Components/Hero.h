//
// Created by Hazen on 12/11/2016.
//

#ifndef LD37_HERO_H
#define LD37_HERO_H

#include <Urho3D/Scene/Component.h>

namespace Urho3D
{
    class Node;
}

namespace Ld37
{
    enum HeroMode {
        MOVING = 0, SEEKING, ATTACKING, DYING
    };

    const static float PATH_TICK = 1.0f;

    /// Component enabling hero behavior
    class Hero : public Urho3D::Component
    {
        URHO3D_OBJECT(Hero, Urho3D::Component);

    public:
        /// Constructor
        Hero(Urho3D::Context* context);

        /// Get's health stat
        int GetHeath() { return health_; }

        /// Get's defense stat
        int GetDefense() { return defense_; }

        /// Get's attack stat
        int GetAttack() { return attack_; }

    protected:
        /// Handle node being assigned
        virtual void OnNodeSet(Urho3D::Node* node);

    private:
        /// Handle frame update events
        void HandleUpdate(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// Health of the hero
        int health_;

        /// Defense characteristic of the hero
        int defense_;

        /// Attack characteristic of the hero
        int attack_;

        /// Time since the last path refresh
        float pathTick_ = 0.f;

        /// Current activity of the hero
        HeroMode currentMode_ = MOVING;
    };
}


#endif //LD37_HERO_H
