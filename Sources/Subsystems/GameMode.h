//
// Created by Hazen Johnson on 12/10/2016.
//

#ifndef LD37_GAMEMODE_H
#define LD37_GAMEMODE_H

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/IO/Log.h>
#include "Subsystems/Mode.h"

namespace Ld37
{
    /// A system for managing different game modes
    class GameMode: public Urho3D::Object
    {
        URHO3D_OBJECT(GameMode, Object);

    public:
        /// Construct
        GameMode(Urho3D::Context* context) : Urho3D::Object(context), context_(context) {}

        /// Destruct
        virtual ~GameMode() {}

        /// Initializes the game mode system with a given game mode
        template <class T>
        void Initialize()
        {
            using namespace Urho3D;
            Log::Write(LOG_INFO, "Initializing GameMode subsystem");
            RegisterMode<T>();
            currentMode_ = T::GetTypeStatic();
            Log::Write(LOG_INFO, "Registered initial game mode " + T::GetTypeNameStatic());

            Mode* currentMode = GetCurrentMode();
            if (currentMode != NULL)
            {
                currentMode->Start();
                Log::Write(LOG_INFO, "Started initial game mode " + currentMode->GetTypeName());
            }

            SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameMode, Update));
            Log::Write(LOG_INFO, "Subscribed to Update event");
        }

        /// Stop all updates to the game mode system
        void Stop()
        {
            using namespace Urho3D;
            UnsubscribeFromAllEvents();
            Log::Write(LOG_INFO, "Unsubscribed from all event");
        }

        /// Registers a game mode
        /// Returns a boolean indicating if the game mode was succesfully created
        template <class T>
        bool RegisterMode()
        {
            using namespace Urho3D;
            if (!gameModes_.Contains(T::GetTypeStatic()))
            {
                Log::Write(LOG_INFO, "Creating new game mode " + T::GetTypeNameStatic());
                Mode* gameMode(new T(context_));
                gameModes_[gameMode->GetType()] = gameMode;
                return true;
            }
            else
            {
                Log::Write(LOG_INFO, "Game mode already exists " + T::GetTypeNameStatic());
                return false;
            }
        }

        /// Moves from the current game mode to another
        /// Requires both game modes to be registered
        template <class T>
        void Next()
        {
            using namespace Urho3D;
            Mode* currentMode = GetCurrentMode();
            Mode* newMode = GetMode(T::GetTypeStatic());

            if (currentMode != NULL)
            {
                Log::Write(LOG_INFO, "Stopping current game mode " + currentMode->GetTypeName());
                currentMode->Stop();
            }
            if (newMode != NULL)
            {
                Log::Write(LOG_INFO, "Setting up new game mode " + newMode->GetTypeName());
                UnsubscribeFromEvent(E_UPDATE);
                SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameMode, StartNext));
            }

            Log::Write(LOG_INFO, "Updating current game mode " + T::GetTypeNameStatic());
            currentMode_ = T::GetTypeStatic();
        }

        /// Gets the current game mode
        Mode* GetCurrentMode()
        {
            return GetMode(currentMode_);
        }

        /// Gets a registered mode according to typename
        Mode* GetMode(const Urho3D::StringHash& mode)
        {
            Mode* currentMode = gameModes_[mode];
            return currentMode;
        }

    private:
        /// Event to handle main loop updates
        void Update(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
        {
            using namespace Urho3D;
            using namespace Urho3D::Update;
            Mode* currentMode = GetCurrentMode();
            if (currentMode != NULL)
            {
                currentMode->Update(eventData[P_TIMESTEP].GetFloat());
            }
        }

        /// Starts the next mode
        void StartNext(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
        {
            using namespace Urho3D;
            UnsubscribeFromEvent(E_UPDATE);
            SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameMode, Update));
            if (GetCurrentMode() != NULL)
            {
                Log::Write(LOG_INFO, "Starting new game mode " + GetCurrentMode()->GetTypeName());
                GetCurrentMode()->Start();
            }
        }

        /// The application context
        Urho3D::Context* context_;

        /// Stores registered game modes
        Urho3D::HashMap<Urho3D::StringHash, Mode*> gameModes_;

        /// The current game mode
        Urho3D::StringHash currentMode_;

    };
}

#endif //LD37_GAMEMODE_H
