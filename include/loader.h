#ifndef LOADER_H
#define LOADER_H

#include <string>

class WorldMap;

namespace Loader
{
    void save(const WorldMap*, std::string file_name);
    void save_dialog(const WorldMap*);
    WorldMap* load(std::string file_name);
    WorldMap* load_dialog();
    WorldMap* blank_map();
}

#endif // LOADER_H
