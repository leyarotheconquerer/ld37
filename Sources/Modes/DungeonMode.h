//
// Created by Hazen on 12/10/2016.
//

#ifndef LD37_DUNGEONMODE_H
#define LD37_DUNGEONMODE_H

#include "Subsystems/Mode.h"

namespace Urho3D
{
    class Input;
    class Node;
    class Scene;
    class Text;
    class UIElement;
}

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
        /// Handle key up events
        void HandleKeyUp(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// Handle main menu button events
        void HandleMainMenu(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// Handle post render update events
        void HandlePostRenderUpdate(Urho3D::StringHash type, Urho3D::VariantMap& data);

        /// The application context
        Urho3D::Context* context_;

        /// Scene of the dungeon mode
        Urho3D::Scene* scene_;

        /// Root element of the UI
        Urho3D::SharedPtr<Urho3D::UIElement> uiRoot_;

        /// Camera scene node
        Urho3D::Node* cameraNode_;
        Urho3D::Vector2 cameraOffset_;

        /// Player Item UI Element
        Urho3D::SharedPtr<Urho3D::Text> playerItem_;
        /// Player scene node
        Urho3D::Node* playerNode_;

        /// Hero Health UI Element
        Urho3D::SharedPtr<Urho3D::Text> heroHealth_;
        /// Hero scene node
        Urho3D::Node* heroNode_;

        /// Node to play music and sound effects
        Urho3D::Node* musicNode_;

        /// Stores input
        Urho3D::Input* input_;
    };
}


#endif //LD37_DUNGEONMODE_H
