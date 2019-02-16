#ifndef GAMEOBJECTARRAY_H
#define GAMEOBJECTARRAY_H

#include "gameobject.h"

class GameObjectArray
{
public:
    GameObjectArray();
    ~GameObjectArray();
    void push_object(std::unique_ptr<GameObject> obj);
    GameObject* operator[](int id);

private:
    std::vector<std::unique_ptr<GameObject>> array_;
};

#endif // GAMEOBJECTARRAY_H
