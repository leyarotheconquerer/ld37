//
// Created by Hazen on 12/10/2016.
//

#ifndef LD37_DUNGEONMODE_H
#define LD37_DUNGEONMODE_H

#include "Subsystems/Mode.h"

namespace Ld37
{
    /// Dungeon game mode
    class DungeonMode : public Mode
    {
        URHO3D_OBJECT(DungeonMode, Mode);

    public:
        /// Constructor
        DungeonMode(Urho3D::Context* context);

        /// Destructor
        virtual ~DungeonMode();

        /// Implement the Start event
        virtual void Start();

        /// Implement the Update event
        virtual void Update(float timestep);

        /// Implement the Stop event
        virtual void Stop();

    private:
        /// Handle pause input
        void HandlePause(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// The application context
        Urho3D::Context* context_;

        /// Scene of the dungeon mode
        Urho3D::Scene* scene_;

        /// Root element of the UI
        Urho3D::SharedPtr<Urho3D::UIElement> uiRoot_;

        /// Node to play music and sound effects
        Urho3D::Node* musicNode_;
    };
}


#endif //LD37_DUNGEONMODE_H
