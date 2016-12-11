//
// Created by Hazen on 12/10/2016.
//

#include "Map.h"
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

using namespace Ld37;
using namespace Urho3D;

static const int EMPTY = 0;
static const int CHECKPOINT = -2;
static const int EXITPOINT = 1;
static const int SPAWNPOINT = -3;

struct ExpectedPoint {
    IntVector2 point;
    int dist;
};

Map::Map(Urho3D::Context* context) :
    Object(context),
    context_(context)
{
}

Map::~Map()
{
}

void logMap(Log* log, int level, Vector<int> map, int width, int height) {
    String mapString("\n");
    for(int i = 0; i < height; i++ )
    {
        for(int j = 0; j < width; j++)
        {
            mapString.AppendWithFormat("\t%d", map[i * width + j]);
        }
        mapString.AppendWithFormat("\n");
    }
    log->Write(level, mapString);
}

Node* Map::Generate(Scene* scene) {

    GenerateMap();

    return ConstructMap(scene);
}

void Map::GenerateMap()
{
    Log *log = GetSubsystem<Log>();

    SetRandomSeed(0);
    int minSize = 1, maxSize = 30;
    float checkpointDensity = .05;

    // This is a super efficient doom counter
    Vector<int> map;
    IntVector2 size;
    int doomCounter = 3;
    int preDoom = doomCounter + 1;
    while (doomCounter != preDoom)
    {
        preDoom = doomCounter;
        // Generate a randomly sized map
        size = IntVector2({
                            (Rand() % (maxSize - minSize)) + minSize,
                            (Rand() % (maxSize - minSize)) + minSize
                        });
        map = Vector<int>(size.x_ * size.y_, EMPTY);
        log->Write(LOG_INFO, String("Generating map")
            .AppendWithFormat(" (%d, %d)", size.x_, size.y_)
            .Append("\n======================================="));

        // Generate an exit point (0-3, NESW)
        int side = Rand() % 4;
        IntVector2 exit = GenerateSidePoint(size, side);
        map[exit.y_ * size.x_ + exit.x_] = EXITPOINT;
        log->Write(LOG_DEBUG, String("Exit point at")
            .AppendWithFormat(" (%d, %d)", exit.x_, exit.y_));

        // Generate a player spawn point on a different side (0-3, NESW)
        int playerSide = (Rand() % 3 + side + 1) % 4;
        IntVector2 spawn = GenerateSidePoint(size, playerSide);
        if (exit == spawn)
        {
            doomCounter--;
            continue;
        }
        else
        {
            map[spawn.y_ * size.x_ + spawn.x_] = SPAWNPOINT;
            log->Write(LOG_DEBUG, String("Spawn point at")
                .AppendWithFormat(" (%d, %d)", exit.x_, exit.y_));
        }

        // Generate a number of expected points in proportion to the checkpoint density
        int expectedCount = (int) ((size.x_ * size.y_) * checkpointDensity + 1);
        log->Write(LOG_DEBUG, String("Expected points")
            .AppendWithFormat(" = %d", expectedCount));
        // Always generate at least one point
        assert(expectedCount > 0);
        Vector<ExpectedPoint> expectedPoints(expectedCount);
        for (int i = 0; i < expectedCount; i++) {
            IntVector2 p({
                             Rand() % size.x_,
                             Rand() % size.y_
                         });
            IntVector2 d({
                             exit.x_ - p.x_,
                             exit.y_ - p.y_
                         });
            expectedPoints[i] = {
                p, (int) Sqrt<float>(d.x_ * d.x_ + d.y_ * d.y_)
            };
            if (map[p.y_ * size.x_ + p.x_] == EMPTY) {
                map[p.y_ * size.x_ + p.x_] = CHECKPOINT;
                log->Write(LOG_DEBUG, String("Check point at")
                    .AppendWithFormat(" (%d, %d)", p.x_, p.y_));
            } else {
                log->Write(LOG_DEBUG, String("Failure point at")
                    .AppendWithFormat(" (%d, %d)", p.x_, p.y_));
            }
        }

        // Sort the checkpoints by distance from the exit
        // Look at me using an inline comparison function
        // FIRST!!!
        // Woohoo!
        Sort(expectedPoints.Begin(), expectedPoints.End(),
             [](const ExpectedPoint& a, const ExpectedPoint& b){
                 return a.dist < b.dist;
             }
        );
        expectedPoints.Insert(expectedPoints.Begin(), {exit, 0});

        logMap(log, LOG_DEBUG, map, size.x_, size.y_);

        // Iterate through checkpoints and generate a path through each one
        IntVector2 previous = expectedPoints.Front().point;
        IntVector2 final = previous;
        expectedPoints.Erase(expectedPoints.Begin()); // TODO: Optimize if Erase takes too much time
        for(auto i = expectedPoints.Begin(); i != expectedPoints.End(); ++i) {
            log->Write(LOG_DEBUG, String(i->dist));
            Vector<IntVector2> path;
            if (AStar(map, size, previous, i->point, path))
            {
                String pathString("Travelling from");
                pathString.AppendWithFormat(" (%d, %d) to (%d, %d)\n",
                                            previous.x_, previous.y_,
                                            i->point.x_, i->point.y_
                );
                for(auto j = path.Begin(); j != path.End(); j++)
                {
                    pathString.AppendWithFormat("(%d, %d)\n", j->x_, j->y_);
                    // Update the count of all rooms after the initial room
                    if (*j != previous)
                    {
                        map[j->y_ * size.x_ + j->x_] = map[previous.y_ * size.x_ + previous.x_] + 1;
                        previous = *j;
                    }
                }
                log->Write(LOG_DEBUG, pathString);

                final = path.Back();
            }
            else {
                log->Write(LOG_DEBUG, String("Could not find a path from")
                    .AppendWithFormat(" (%d, %d) to (%d, %d)\n",
                                      previous.x_, previous.y_,
                                      i->point.x_, i->point.y_
                    )
                );
                map[i->point.y_ * size.x_ + i->point.x_] = EMPTY;
            }
        }

        if (doomCounter <= 0)
        {
            log->Write(LOG_ERROR, "You have successfully managed to generate an impossible level.");
            log->Write(LOG_ERROR, "And you did this not just once, but three different times.");
            log->Write(LOG_ERROR, "Congratulations!");
            log->Write(LOG_ERROR, "Way to confound the RNG system.");
            GetSubsystem<Engine>()->Exit();
        }

        logMap(log, LOG_INFO, map, size.x_, size.y_);
        log->Write(LOG_INFO, "=======================");
    }

    map_ = map;
    size_ = size;
}

Urho3D::Node* Map::ConstructMap(Scene* scene)
{
    const float TILE_SIZE = 128 * PIXEL_SIZE;
    const float ROOM_SIZE = TILE_SIZE * 4;
    const int Z_OFFSET = 10 * PIXEL_SIZE;
    const String CLOSED_ROOM = "Closed";
    const String OPEN_ROOM = "Open";
    const String FLOOR = "Floor";

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // And now I'm using initializer lists
    // I'm so 11 :)
    // Also, my IDE is annoyed with me
    HashMap<int, IntVector2> directions = {
        {0, {0, 1}},   {1, {0, 1}},	{2, {0, 1}},	{3, {0, 1}},
        {4, {-1, 0}},   {5, {-1, 0}},	{6, {1, 0}},	{7, {1, 0}},
        {8, {-1, 0}},   {9, {-1, 0}},	{10, {1, 0}},	{11, {1, 0}},
        {12, {0, -1}},   {13, {0, -1}},   {14, {0, -1}},	{15, {0, -1}}
    };

    Node* mapNode = scene->CreateChild("Map");
    SpriteSheet2D* levelSheet = cache->GetResource<SpriteSheet2D>("Textures/LevelAssetsSheet.xml");

    // Generate all rooms in the map
    for (int x = 0; x < size_.x_; x++)
    {
        for (int y = 0; y < size_.y_; y++)
        {
            if (map_[y * size_.x_ + x] != EMPTY)
            {
                String roomName;
                roomName.AppendWithFormat("Room[%d,%d]", x, y);
                Node* roomNode = mapNode->CreateChild(roomName);
                Node* floorNode = roomNode->CreateChild("Floor");
                Node* wallNode = roomNode->CreateChild("Walls");
                for (int i = 0; i < 16; i++)
                {
                    String spriteName;
                    IntVector2 dir = directions[i];
                    // Determine whether there is an adjacent room is in this direction
                    int neighbor = (y + dir.y_) * size_.x_ + x + dir.x_;
                    if (neighbor >= 0 && neighbor < map_.Size() &&
                        map_[neighbor] > 0 &&
                        (map_[y * size_.x_ + x] == map_[neighbor] + 1 ||
                        map_[y * size_.x_ + x] == map_[neighbor] - 1
                        ))
                    {
                        spriteName = OPEN_ROOM;
                    }
                    else
                    {
                        spriteName = CLOSED_ROOM;
                    }
                    spriteName.AppendWithFormat("%d", i);

                    Node* sectionNode = wallNode->CreateChild(spriteName);
                    sectionNode->SetPosition(Vector3(
                        x * ROOM_SIZE + (i % 4) * TILE_SIZE,
                        y * ROOM_SIZE + ((15 - i) / 4) * TILE_SIZE
                    ));
                    Sprite2D* sectionSprite = levelSheet->GetSprite(spriteName);
                    StaticSprite2D* spriteComponent = sectionNode->CreateComponent<StaticSprite2D>();
                    spriteComponent->SetSprite(sectionSprite);
                    spriteComponent->SetLayer(i / 8 + 1);

                    String floorName = FLOOR;
                    floorName.AppendWithFormat("%d", i);
                    sectionNode = floorNode->CreateChild(floorName);
                    sectionNode->SetPosition(Vector3(
                        x * ROOM_SIZE + (i % 4) * TILE_SIZE,
                        y * ROOM_SIZE + (i / 4) * TILE_SIZE
                    ));
                    sectionSprite = levelSheet->GetSprite(floorName);
                    spriteComponent = sectionNode->CreateComponent<StaticSprite2D>();
                    spriteComponent->SetSprite(sectionSprite);
                    spriteComponent->SetLayer(0);
                }
            }
        }
    }

    return mapNode;
}

Urho3D::IntVector2 Map::GenerateSidePoint(Urho3D::IntVector2 size, int side)
{
    IntVector2 point;
    switch (side) {
        case 0:
            point.x_ = Rand() % size.x_;
            point.y_ = 0;
            break;
        case 1:
            point.x_ = size.x_ - 1;
            point.y_ = Rand() % size.y_;
            break;
        case 2:
            point.x_ = Rand() % size.x_;
            point.y_  = size.y_ - 1;
            break;
        case 3:
            point.x_ = 0;
            point.y_ = Rand() % size.y_;
            break;
        default:
            assert(false); // Never reach this
            break;
    }
    return point;
}

// Implemented based on wikipedia
// https://en.wikipedia.org/wiki/A*_search_algorithm
bool Map::AStar(Urho3D::Vector<int>& map, Urho3D::IntVector2 size,
    Urho3D::IntVector2 start, Urho3D::IntVector2 end,
    Urho3D::Vector<Urho3D::IntVector2>& path)
{
    HashSet<IntVector2> closedSet;
    Vector<IntVector2> openSet;
    openSet.Push(start);

    HashMap<IntVector2, IntVector2> cameFrom;
    /// Cost of getting to the node
    HashMap<IntVector2, int> gScore;
    gScore[start] = 0;

    HashMap<IntVector2, int> fScore;
    fScore[start] = (int)(end - start).Length();

    while(openSet.Size() > 0)
    {
        Sort(openSet.Begin(), openSet.End(),
            [end](const IntVector2& a, const IntVector2& b){
                int distA = (int)(end - a).Length();
                int distB = (int)(end - b).Length();
                return distA > distB;
            }
        );
        IntVector2 current = openSet.Back();
        if (current == end)
        {
            ConstructPath(cameFrom, current, path);
            return true;
        }
        openSet.Pop();
        closedSet.Insert(current);

        Vector<IntVector2> neighbors;
        if (current.y_ > 0) // Up
            neighbors.Push(IntVector2(current.x_, current.y_ - 1));
        if (current.x_ < size.x_ - 1) // Right
            neighbors.Push(IntVector2(current.x_ + 1, current.y_));
        if (current.y_ < size.y_ - 1) // Down
            neighbors.Push(IntVector2(current.x_, current.y_ + 1));
        if (current.x_ > 0) // Left
            neighbors.Push(IntVector2(current.x_ - 1, current.y_));

        for(auto i = neighbors.Begin(); i != neighbors.End(); ++i)
        {
            if (closedSet.Contains(*i) ||
                (map[i->y_ * size.x_ + i->x_] != EMPTY && *i != end))
            {
                continue;
            }
            int tentative_gScore = gScore[current] + 1;
            if (!openSet.Contains(*i))
            {
                openSet.Push(*i);
            }
            else if (gScore.Contains(*i) && tentative_gScore >= gScore[*i])
            {
                continue;
            }

            cameFrom[*i] = current;
            gScore[*i] = tentative_gScore;
            fScore[*i] = tentative_gScore + (int)(end - (*i)).Length();
        }
    }

    // If we reach this, no path was found
    return false;
}

void Map::ConstructPath(const Urho3D::HashMap<Urho3D::IntVector2, Urho3D::IntVector2> &mappings,
    Urho3D::IntVector2 current,
    Urho3D::Vector<Urho3D::IntVector2>& path)
{
    path.Push(current);
    while (mappings.Contains(current))
    {
        current = *mappings[current];
        path.Push(current);
    }

    // Reverse the backwards path
    for(int i = 0; i < path.Size() / 2; i++)
    {
        IntVector2 temp = path[i];
        path[i] = path[path.Size() - i - 1];
        path[path.Size() - i - 1] = temp;
    }
}
