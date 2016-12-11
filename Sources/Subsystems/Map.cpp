//
// Created by Hazen on 12/10/2016.
//

#include "Map.h"
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/DebugNew.h>

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

    Vector<int> map = GenerateMap();

    PopulateMap(map);

    return ConstructMap(scene);
}

Urho3D::Vector<int> Map::GenerateMap()
{
    Log *log = GetSubsystem<Log>();

    SetRandomSeed(0);

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
                            (Rand() % (MAX_MAP_SIZE - MIN_MAP_SIZE)) + MIN_MAP_SIZE,
                            (Rand() % (MAX_MAP_SIZE - MIN_MAP_SIZE)) + MIN_MAP_SIZE
                        });
        map = Vector<int>(size.x_ * size.y_, EMPTY);
        log->Write(LOG_INFO, String("Generating map")
            .AppendWithFormat(" (%d, %d)", size.x_, size.y_)
            .Append("\n======================================="));

        // Generate an exit point (0-3, NESW)
        int side = Rand() % 4;
        IntVector2 exit = GenerateSidePoint(size, side);
        heroExit_ = exit.y_ * size.x_ + exit.x_;
        map[heroExit_] = EXITPOINT;
        log->Write(LOG_DEBUG, String("Exit point at")
            .AppendWithFormat(" (%d, %d)", exit.x_, exit.y_));

        // Generate a player spawn point on a different side (0-3, NESW)
        int playerSide = (Rand() % 3 + side + 1) % 4;
        IntVector2 spawn = GenerateSidePoint(size, playerSide);
        playerSpawn_ = spawn.y_ * size.x_ + spawn.x_;
        if (heroExit_ == playerSpawn_)
        {
            doomCounter--;
            continue;
        }
        else
        {
            map[playerSpawn_] = SPAWNPOINT;
            log->Write(LOG_DEBUG, String("Spawn point at")
                .AppendWithFormat(" (%d, %d)", exit.x_, exit.y_));
        }

        // Generate a number of expected points in proportion to the checkpoint density
        int expectedCount = (int) ((size.x_ * size.y_) * CHECKPOINT_DENSITY + 1);
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
        heroSpawn_ = heroExit_;
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

                IntVector2 initial = path.Back();
                heroSpawn_ = initial.y_ * size.x_ + initial.x_;
                heroPathLength_ = map[heroSpawn_];
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

    size_ = size;
    return map;
}

void Map::PopulateMap(Urho3D::Vector<int>& map)
{
    Log* log = GetSubsystem<Log>();

    /// Relative locations to place items in a room (0-4 Center,NESW)
    Vector<Vector2> placements = {
        {TILE_SIZE * 1.5f, TILE_SIZE * 1.5f},
        {TILE_SIZE * 1.5f, TILE_SIZE * 2.f},
        {TILE_SIZE * 2.5f, TILE_SIZE * 1.5f},
        {TILE_SIZE * 1.5f, TILE_SIZE * .5f},
        {TILE_SIZE * .5f, TILE_SIZE * 1.5f},
    };

    map_ = Vector<Space>(map.Size());
    for (int i = 0; i < map.Size(); ++i)
    {
        IntVector2 space({
                             i % size_.x_,
                             i / size_.x_
                         });
        Vector2 pos = Vector2(space.x_ * ROOM_SIZE, space.y_ * ROOM_SIZE);
        if (map[i] == EMPTY)
        {
            map_[i].type = Space::EMPTY;
            map_[i].pos = pos;
        }
        else if(map[i] == SPAWNPOINT)
        {
            map_[i].type = Space::SPAWN;
            map_[i].pos = pos;
            playerSpawn_ = i;
        }
        else
        {
            map_[i].type = Space::ROOM;
            map_[i].pos = pos;
            HashMap<int, int> neighbors = {
                {Space::SOUTH, (space.y_ - 1) * size_.x_ + space.x_},
                {Space::EAST, space.y_ * size_.x_ + space.x_ + 1},
                {Space::NORTH, (space.y_ + 1) * size_.x_ + space.x_},
                {Space::WEST, space.y_ * size_.x_ + space.x_ - 1},
            };
            for (auto j = neighbors.Begin(); j != neighbors.End(); j++)
            {
                if (j->second_ >= 0 && j->second_ < map.Size())
                {
                    int test = map[j->second_];
                    if (map[j->second_] == map[i] + 1)
                    {
                        map_[i].prev = &map_[j->second_];
                        map_[i].prevDir = static_cast<Space::Direction>(j->first_);
                    }
                    else if (map[j->second_] == map[i] - 1 && map[j->second_] != 0)
                    {
                        map_[i].next = &map_[j->second_];
                        map_[i].nextDir = static_cast<Space::Direction>(j->first_);
                    }
                }
            }
            if (i == heroSpawn_)
            {
                Item spawner;
                spawner.type = Item::HERO_SPAWNER;
                spawner.dir = Item::CENTER;
                spawner.pos = pos + placements[spawner.dir];
                map_[i].items.Push(spawner);

                heroSpawn_ = i;
            }
            if (i == heroExit_)
            {
                Item spawner;
                spawner.type = Item::EXIT;
                spawner.dir = Item::CENTER;
                spawner.pos = pos + placements[spawner.dir];
                map_[i].items.Push(spawner);

                heroExit_ = i;
            }
        }
    }

    // Generate player pickups
    int i = 0;
    int placed = 0;
    while (i < size_.x_ * size_.y_ &&
        placed < ITEM_DENSITY * size_.x_ * size_.y_)
    {
        int spaceIndex = Rand() % map_.Size();
        if (map_[spaceIndex].type == Space::EMPTY && map_[spaceIndex].items.Size() == 0)
        {
            IntVector2 space = {
                spaceIndex % size_.x_,
                spaceIndex / size_.x_
            };
            Vector2 pos = Vector2(space.x_ * ROOM_SIZE, space.y_ * ROOM_SIZE);

            Item item;
            item.dir = Item::CENTER;
            // HOLE -> FALSE_TREASURE
            item.type = static_cast<Item::Type>((Rand() % 3) + 1);
            item.pos = pos + placements[item.dir];

            map_[i].items.Push(item);
            placed++;
        }
        i++;
    }

    // Generate hero pickups
    i = 0;
    placed = 0;
    while (i < heroPathLength_ &&
           placed < ITEM_DENSITY * heroPathLength_)
    {
        int spaceIndex = Rand() % map_.Size();
        if (map_[spaceIndex].type == Space::ROOM &&
            !map_[spaceIndex].items.Size() &&
            spaceIndex != heroSpawn_ &&
            spaceIndex != heroExit_)
        {
            IntVector2 space = {
                spaceIndex % size_.x_,
                spaceIndex / size_.x_
            };
            Vector2 pos = Vector2(space.x_ * ROOM_SIZE, space.y_ * ROOM_SIZE);

            Item item;
            item.dir = static_cast<Item::Direction>(map_[spaceIndex].prevDir);
            item.type = Item::TREASURE;
            item.pos = pos + placements[item.dir];

            map_[spaceIndex].items.Push(item);
            placed++;
        }
        i++;
    }
}

Urho3D::Node* Map::ConstructMap(Scene* scene)
{
    Log* log = GetSubsystem<Log>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // And now I'm using initializer lists
    // I'm so 11 :)
    // Also, my IDE is annoyed with me
    HashMap<int, int> directions = {
        {0, Space::NORTH},
        {1, Space::NORTH},
        {2, Space::NORTH},
        {3, Space::NORTH},
        {4, Space::WEST},
        {5, Space::WEST},
        {6, Space::EAST},
        {7, Space::EAST},
        {8, Space::WEST},
        {9, Space::WEST},
        {10, Space::EAST},
        {11, Space::EAST},
        {12, Space::SOUTH},
        {13, Space::SOUTH},
        {14, Space::SOUTH},
        {15, Space::SOUTH},
    };

    Node* mapNode = scene->CreateChild("Map");
    SpriteSheet2D* levelSheet = cache->GetResource<SpriteSheet2D>("Textures/LevelAssetsSheet.xml");

    // Generate all rooms in the map
    for (int x = 0; x < size_.x_; x++)
    {
        for (int y = 0; y < size_.y_; y++)
        {
            int spaceIndex = y * size_.x_ + x;

            String roomName;
            roomName.AppendWithFormat("Room[%d,%d]", x, y);
            Node* roomNode = mapNode->CreateChild(roomName);

            if (map_[spaceIndex].items.Size())
            {
                Node* itemsNode = roomNode->CreateChild("Items");
                for (auto i = map_[spaceIndex].items.Begin(); i != map_[spaceIndex].items.End(); i++)
                {
                    String itemName;
                    switch (i->type)
                    {
                        case Item::TREASURE:
                            itemName = "Chest";
                            if (Rand() % 2) { itemName.Append("V"); }
                            else { itemName.Append("H"); }
                            if (i->triggered) { itemName.Append("Open0"); }
                            else { itemName.Append("Close0"); }
                            break;
                        case Item::HOLE:
                            itemName = "Hole1";
                            break;
                        case Item::MONSTER_SPAWNER:
                            itemName = "Spawner1";
                            break;
                        case Item::FALSE_TREASURE:
                            itemName = "Chest";
                            if (Rand() % 2) { itemName.Append("V"); }
                            else { itemName.Append("H"); }
                            if (i->triggered) { itemName.Append("Open1"); }
                            else { itemName.Append("Close1"); }
                            break;
                        case Item::EXIT:
                        case Item::HERO_SPAWNER:
                            itemName = "Spawner0";
                            break;
                        default:
                            break;
                    }
                    Node* itemNode = itemsNode->CreateChild(itemName);
                    itemNode->SetPosition(Vector3(i->pos.x_, i->pos.y_, 0));
                    Sprite2D* itemSprite = levelSheet->GetSprite(itemName);
                    StaticSprite2D* itemStatic = itemNode->CreateComponent<StaticSprite2D>();
                    itemStatic->SetSprite(itemSprite);
                    itemStatic->SetLayer(11);
                    i->node = itemNode;

                    if (i->type == Item::HOLE)
                    {
                        Node* subItemNode = itemNode->CreateChild("Cover");
                        Sprite2D* subItemSprite = levelSheet->GetSprite("HoleCover1");
                        StaticSprite2D* subItemStatic = subItemNode->CreateComponent<StaticSprite2D>();
                        subItemStatic->SetSprite(subItemSprite);
                        subItemStatic->SetLayer(12);
                    }
                }
            }

            if (map_[spaceIndex].type == Space::ROOM)
            {
                Node* floorNode = roomNode->CreateChild("Floor");
                Node* wallNode = roomNode->CreateChild("Walls");

                Node* collisionNode = floorNode->CreateChild("Collision");
                collisionNode->SetPosition(Vector3(TILE_SIZE + .6 + ROOM_SIZE * x, 1, TILE_SIZE + .2 + ROOM_SIZE * y));
                collisionNode->SetScale(Vector3(1.8, 1.8, 1.8));
                StaticModel* model = collisionNode->CreateComponent<StaticModel>();
                model->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
                collisionNode->CreateComponent<Navigable>();

                for (int i = 0; i < 16; i++)
                {
                    String spriteName;
                    Space::Direction dir = static_cast<Space::Direction>(directions[i]);
                    if(map_[spaceIndex].nextDir == dir ||
                       map_[spaceIndex].prevDir == dir)
                    {
                        if (map_[spaceIndex].nextDir == dir)
                        {
                            Node* doorNode = floorNode->CreateChild("Door");
                            doorNode->SetScale(Vector3(.4f, .6f, 1));
                            Vector3 delta;
                            if (dir == Space::NORTH)
                            {
                                delta = Vector3(1.85f, ROOM_SIZE - 1.f, 0);
                            }
                            else if(dir == Space::SOUTH)
                            {
                                delta = Vector3(1.85f, -1.f, 0);
                            }
                            else if(dir == Space::EAST)
                            {
                                delta = Vector3(ROOM_SIZE - 0.8f, 1.5f, 0);
                                doorNode->Yaw(90);
                            }
                            else if(dir == Space::WEST)
                            {
                                delta = Vector3(-0.8f, 1.5f, 0);
                                doorNode->Yaw(90);
                            }
                            doorNode->SetPosition(Vector3(ROOM_SIZE * x + delta.x_, 1, ROOM_SIZE * y + delta.y_));
                            model = doorNode->CreateComponent<StaticModel>();
                            model->SetModel((cache->GetResource<Model>("Models/Plane.mdl")));
                            doorNode->CreateComponent<Navigable>();
                        }
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
                    spriteComponent->SetLayer((i / 8) * 10 + 10);

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

    navMesh_ = scene->GetOrCreateComponent<NavigationMesh>();
    navMesh_->SetPadding(Vector3(0.0f, 10.0f, 0.0f));
    navMesh_->SetAgentRadius(TILE_SIZE / 4.f);
    navMesh_->SetCellSize(.01f);
    navMesh_->Build();

    navMesh_->FindPath(path_, Vector3(37.12, 1.03, 26.88), Vector3(3.84, 1.03, 1.28));

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
