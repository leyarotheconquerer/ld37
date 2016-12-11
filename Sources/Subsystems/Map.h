//
// Created by Hazen on 12/10/2016.
//

#ifndef LD37_MAP_H
#define LD37_MAP_H

#include <Urho3D/Core/Object.h>

namespace Urho3D
{
    class Node;
}

namespace Ld37
{
    /// Generates and contains all logic concerning the level map
    class Map : public Urho3D::Object {
        URHO3D_OBJECT(Map, Urho3D::Object);

    public:
        /// Constructor
        Map(Urho3D::Context* context);

        /// Destructor
        virtual ~Map();

        /// Generates a new map and returns the root node of the result
        Urho3D::Node* Generate();

    private:
        /// Generates a map
        void GenerateMap();

        /// Constructs a pre-generated map
        Urho3D::Node* ConstructMap();

        /// Generates a point on the side of map
        Urho3D::IntVector2 GenerateSidePoint(Urho3D::IntVector2 size, int side);

        /// Calculates the shortest path between a start and an end point
        /// Returns false if no path is found
        bool AStar(Urho3D::Vector<int> &map, Urho3D::IntVector2 size,
            Urho3D::IntVector2 start, Urho3D::IntVector2 end,
            Urho3D::Vector <Urho3D::IntVector2>& path);

        /// Constructs a path based on the output of the AStar algorithm
        /// A utility function called by AStar in the final step
        void ConstructPath(const Urho3D::HashMap<Urho3D::IntVector2, Urho3D::IntVector2>& mappings,
            Urho3D::IntVector2 current,
            Urho3D::Vector<Urho3D::IntVector2>& path);

        /// The application context
        Urho3D::Context* context_;

        /// A numerical representation of the map
        Urho3D::Vector<int> map_;
        Urho3D::IntVector2 size_;

        /// List of nodes that make up the map
        Urho3D::Vector<Urho3D::SharedPtr<Urho3D::Node> > nodes_;
    };
}


#endif //LD37_MAP_H
