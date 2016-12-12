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
    struct Item;
    enum HeroMode {
        MOVING = 0, SEEKING, ATTACKING, DYING
    };

    const float UPDATE_RATE = 1.0f;
    const float HERO_MOVEMENT_RATE = 1.0f;

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

        /// Resolves an attack against the hero
        void ResolveAttack(int min, int max);

        /// Reasolves a treasure bonus
        void ResolveHeal(int minHeal, int maxHeal);

        /// Health of the hero
        int health_;

        /// Defense characteristic of the hero
        int defense_;

        /// Attack characteristic of the hero
        int attack_;

    public:
        /// The hero's journey
        Urho3D::PODVector<Urho3D::Vector2> currentPath_;
    private:

        /// Time since the last path refresh
        float updateTick_ = 0.f;

        /// Current activity of the hero
        HeroMode currentMode_ = MOVING;

        /// The current item of the hero's focus
        Item* currentItem_;
    };
}


#endif //LD37_HERO_H
