#ifndef LOADER_H
#define LOADER_H

class WorldMap;

namespace Loader
{
    void save(const WorldMap* world_map);
    WorldMap* load();
    WorldMap* blank_map();
}

#endif // LOADER_H
