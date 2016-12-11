//
// Created by Hazen on 12/10/2016.
//

#include "Map.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Node.h>

using namespace Ld37;
using namespace Urho3D;

static const int EMPTY = 0;
static const int CHECKPOINT = 1;
static const int EXITPOINT = 2;
static const int SPAWNPOINT = 3;
static const int ROOMPOINT = 4;

struct ExpectedPoint {
    IntVector2 point;
    int dist;
};
bool CompareExpectedPoints(const ExpectedPoint& a, const ExpectedPoint& b)
{
    return a.dist < b.dist;
}

Map::Map(Urho3D::Context *context) :
    Object(context),
    context_(context)
{
}

Map::~Map()
{
}

void logMap(Log* log, Vector<int> map, int width, int height) {
    String mapString("\n");
    for(int i = 0; i < height; i++ )
    {
        for(int j = 0; j < width; j++)
        {
            mapString.AppendWithFormat("\t%d", map[i * width + j]);
        }
        mapString.AppendWithFormat("\n");
    }
    log->Write(LOG_INFO, mapString);
}

Node* Map::Generate() {
    Log *log = GetSubsystem<Log>();

    SetRandomSeed(0);
    int minSize = 1, maxSize = 30;
    float checkpointDensity = .05;

    // Generate a randomly sized map
    IntVector2 size({
        (Rand() % (maxSize - minSize)) + minSize,
        (Rand() % (maxSize - minSize)) + minSize
    });
    Vector<int> map(size.x_ * size.y_, EMPTY);
    log->Write(LOG_INFO, String("Generating map")
        .AppendWithFormat(" (%d, %d)", size.x_, size.y_)
        .Append("\n======================================="));

    // Generate an exit point (0-3, NESW)
    int side = Rand() % 4;
    IntVector2 exit;
    switch (side) {
        case 0:
            exit.x_ = Rand() % size.x_;
            exit.y_ = 0;
            break;
        case 1:
            exit.x_ = size.x_ - 1;
            exit.y_ = Rand() % size.y_;
            break;
        case 2:
            exit.x_ = Rand() % size.x_;
            exit.y_  = size.y_ - 1;
            break;
        case 3:
            exit.x_ = 0;
            exit.y_ = Rand() % size.y_;
            break;
        default:
            assert(false); // Never reach this
            break;
    }
    map[exit.y_ * size.x_ + exit.x_] = EXITPOINT;
    log->Write(LOG_INFO, String("Exit point at")
        .AppendWithFormat(" (%d, %d)", exit.x_, exit.y_));

    // Generate a number of expected points in proportion to the checkpoint density
    int expectedCount = (int) ((size.x_ * size.y_) * checkpointDensity + 1);
    log->Write(LOG_INFO, String("Expected points")
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
            log->Write(LOG_INFO, String("Check point at")
                .AppendWithFormat(" (%d, %d)", p.x_, p.y_));
        } else {
            log->Write(LOG_INFO, String("Failure point at")
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
    expectedPoints.Push({exit, 0});

    logMap(log, map, size.x_, size.y_);

    // Iterate through checkpoints and generate a path through each one
    IntVector2 initial = expectedPoints.Front().point;
    IntVector2 previous = initial;
    expectedPoints.Erase(expectedPoints.Begin()); // TODO: Optimize if Erase takes too much time
    for(auto i = expectedPoints.Begin(); i != expectedPoints.End(); ++i) {
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
            log->Write(LOG_INFO, pathString);

        }
        else {
            log->Write(LOG_INFO, String("Could not find a path from")
                .AppendWithFormat(" (%d, %d) to (%d, %d)\n",
                    previous.x_, previous.y_,
                    i->point.x_, i->point.y_
                )
            );
        }
    }

    logMap(log, map, size.x_, size.y_);
    log->Write(LOG_INFO, "=======================");

    return (new Node(context_));
}

#include <iostream>
using namespace std;

void printVector(Vector<IntVector2>& vector)
{
    for(auto i = vector.Begin(); i != vector.End(); i++)
    {
        cout << "(" << i->x_ << ", " << i->y_ << ")" << endl;
    }
}

// Implemented based on wikipedia
// https://en.wikipedia.org/wiki/A*_search_algorithm
bool Map::AStar(Vector<int>& map, IntVector2 size,
    IntVector2 start, IntVector2 end,
    Vector<IntVector2>& path)
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
    IntVector2 current,
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
