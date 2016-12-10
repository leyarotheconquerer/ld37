//
// Created by Hazen on 12/10/2016.
//

#ifndef LD37_MAINMENUMODE_H
#define LD37_MAINMENUMODE_H

#include "Subsystems/Mode.h"

namespace Urho3D
{
    class Scene;
    class Node;
    class UIElement;
}

namespace Ld37
{
    /// Main menu game mode
    class MainMenuMode : public Mode
    {
        URHO3D_OBJECT(MainMenuMode, Mode);

    public:
        /// Constructor
        MainMenuMode(Urho3D::Context* context);

        /// Destructor
        virtual ~MainMenuMode();

        /// Implement the Start event
        virtual void Start();

        /// Implement the Update event
        virtual void Update(float timestep);

        /// Implement the Stop event
        virtual void Stop();

    private:
        /// Handle presses of the Play button
        void HandlePlay(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// Handle exit events
        void HandleExit(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// The application context
        Urho3D::Context* context_;

        /// Scene of the current mode
        Urho3D::Scene* scene_;

        /// Root element of the UI
        Urho3D::SharedPtr<Urho3D::UIElement> uiRoot_;

        /// Node which plays music
        Urho3D::Node* musicNode_;
    };
}

#endif //LD37_MAINMENUMODE_H
