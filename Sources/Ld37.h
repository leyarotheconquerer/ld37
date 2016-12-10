//
// Created by Hazen on 12/10/2016.
//

#ifndef LD37_LD37APP_H
#define LD37_LD37APP_H

#include <Urho3D/Engine/Application.h>

namespace Ld37 {
    /// Ld37 application class
    class Ld37App : public Urho3D::Application {
        URHO3D_OBJECT(Ld37App, Urho3D::Application);

    public:
        /// Constructor
        Ld37App(Urho3D::Context *context);

        /// Destrutor
        virtual ~Ld37App() {}

        /// Implement the Setup event
        virtual void Setup();

        /// Implement the Start event
        virtual void Start();

        /// Implement the Stop event
        virtual void Stop();

    protected:

    private:
        /// Application context
        Urho3D::Context *context_;

    };
}


#endif //LD37_LD37APP_H
