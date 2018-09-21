#ifndef LOADER_H
#define LOADER_H

#include <memory>
#include <string>

class RoomMap;

namespace Loader
{
    void save(const RoomMap*, std::string file_name);
    void save_dialog(const RoomMap*);
    std::unique_ptr<RoomMap> load(std::string file_name);
    std::unique_ptr<RoomMap> load_dialog();
    std::unique_ptr<RoomMap> blank_map();
}

#endif // LOADER_H
