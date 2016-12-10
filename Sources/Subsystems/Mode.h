/// By Hazen Johnson
/// Copyright (c) 2016
/// MIT License
#ifndef Mode_H
#define Mode_H

#include <Urho3D/Core/Object.h>

namespace Ld37
{

    class Mode : public Urho3D::Object
    {

    public:
        Mode(Urho3D::Context* context) : Urho3D::Object(context) {}

        virtual void Start() = 0;
        virtual void Update(float timestep) = 0;
        virtual void Stop() = 0;
    };

}

#endif //Mode_H
