//
// Created by Hazen on 12/10/2016.
//

#ifndef LD37_MAP_H
#define LD37_MAP_H

#include <Urho3D/Core/Object.h>
#include <Urho3D/Urho2D/Drawable2D.h>

namespace Urho3D
{
    class Node;
    class Scene;
    class NavigationMesh;
}

namespace Ld37
{
    /// Minimum size of a generated map
    const int MIN_MAP_SIZE = 1;
    /// Maximum size of a generated map
    const int MAX_MAP_SIZE = 30;
    /// Density of checkpoints in the hero path relative to map size
    const float CHECKPOINT_DENSITY = .05;
    /// Density of item spawn relative to map size
    const float ITEM_DENSITY = .25;
    /// Size of a room tile
    const float TILE_SIZE = 128 * Urho3D::PIXEL_SIZE;
    /// Size of a room
    const float ROOM_SIZE = TILE_SIZE * 4;
    /// Name of a closed room tile
    const Urho3D::String CLOSED_ROOM = "Closed";
    /// Name of a open room tile
    const Urho3D::String OPEN_ROOM = "Open";
    /// Name of a floor room tile
    const Urho3D::String FLOOR = "Floor";

    // Item attack rates, etc...
    // Should this be it's own component?
    // Yes
    // Is it?
    // Nah. time constraints.
    // Thanks Ludum Dare
    const int HOLE_MIN_ATTACK = 5;
    const int HOLE_MAX_ATTACK = 50;
    const int FALSE_TREASURE_MIN_ATTACK = 4;
    const int FALSE_TREASURE_MAX_ATTACK = 20;
    const float MONSTER_SPAWN_RATE = 5.f;
    const float MONSTER_SPAWN_COUNT = 5.f;
    const int MONSTER_MIN_HEALTH = 4;
    const int MONSTER_MAX_HEALTH = 10;
    const int MONSTER_MIN_ATTACK = 6;
    const int MONSTER_MAX_ATTACK = 20;
    const int TREASURE_MIN_HEAL = 10;
    const int TREASURE_MAX_HEAL = 30;

    /// Stores information about items in a room
    struct Item {
        enum Type {
            TREASURE = 0, HOLE = 1, MONSTER_SPAWNER, FALSE_TREASURE, EXIT, HERO_SPAWNER
        } type;
        enum Direction {
            CENTER = 0, NORTH, EAST, SOUTH, WEST
        } dir;
        Urho3D::Vector2 pos;
        Urho3D::Node* node;
        bool triggered = false;
    };

    /// Stores information about rooms in a map
    struct Space {
        enum Type {
            ROOM, EMPTY, SPAWN
        } type;
        enum Direction {
            NO_DIR = 0, NORTH, EAST, SOUTH, WEST
        };
        Urho3D::IntVector2 idx;
        Urho3D::Vector2 pos;
        Space* next = NULL;
        Direction nextDir = NO_DIR;
        Space* prev = NULL;
        Direction prevDir = NO_DIR;
        Urho3D::Vector<Item> items;
    };
    /// Generates and contains all logic concerning the level map
    class Map : public Urho3D::Object {
        URHO3D_OBJECT(Map, Urho3D::Object);

    public:
        /// Constructor
        Map(Urho3D::Context* context);

        /// Destructor
        virtual ~Map();

        /// Generates a new map and returns the root node of the result
        Urho3D::Node* Generate(Urho3D::Scene* scene_);

        /// Get the space at the given indices
        Space* GetSpaceIndex(const Urho3D::IntVector2 i)
        {
            return &map_[i.y_ * size_.x_ + i.x_];
        }

        /// Get the space at the given world coordinates
        Space* GetSpaceWorld(const Urho3D::Vector2 i)
        {
            Urho3D::IntVector2 pos = {
                (int)(i.x_ / ROOM_SIZE),
                (int)(i.y_ / ROOM_SIZE),
            };
            return GetSpaceIndex(pos);
        }

        /// Get the hero's spawn space
        Space* GetHeroSpawn()
        {
            return &map_[heroSpawn_];
        }

        /// Get the hero's exit space
        Space* GetHeroExit()
        {
            return &map_[heroExit_];
        }

        /// Get the player's spawn space
        Space* GetPlayerSpawn()
        {
            return &map_[playerSpawn_];
        }

        /// Gets a list of points forming a path from the start to the end
        /// List is empty if no path exists
        Urho3D::PODVector<Urho3D::Vector2> GetPath(Urho3D::Vector2 start, Urho3D::Vector3 end);

        /// Temp path debugger
        Urho3D::PODVector<Urho3D::Vector3> path_;

    private:
        /// Generates a map
        Urho3D::Vector<int> GenerateMap();

        /// Populates a map with items
        void PopulateMap(Urho3D::Vector<int>& map);

        /// Constructs a pre-generated map
        Urho3D::Node* ConstructMap(Urho3D::Scene* scene);

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

        /// The map data of the map
        Urho3D::Vector<Space> map_;
        Urho3D::IntVector2 size_;

        int heroSpawn_;
        int heroExit_;
        int playerSpawn_;
        int heroPathLength_;

        /// List of nodes that make up the map
        Urho3D::Node* mapNode_;
        Urho3D::NavigationMesh* navMesh_;

    };
}


#endif //LD37_MAP_H
