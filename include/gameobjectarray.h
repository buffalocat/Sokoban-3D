#ifndef GAMEOBJECTARRAY_H
#define GAMEOBJECTARRAY_H

#include <memory>
#include <vector>

class GameObject;

class GameObjectArray
{
public:
    GameObjectArray();
    ~GameObjectArray();
    void push_object(std::unique_ptr<GameObject> obj);
    GameObject* operator[](int id);
    GameObject* safe_get(int id);
    void destroy(GameObject* obj);

private:
    std::vector<std::unique_ptr<GameObject>> array_;
};

#endif // GAMEOBJECTARRAY_H
