#ifndef DOORTAB_H
#define DOORTAB_H

#include <string>


#include "editortab.h"
#include "point.h"

class Door;

class DoorTab: public EditorTab {
public:
    DoorTab(EditorState*, GraphicsManager*);
    ~DoorTab();

    void init();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);

private:
    Door* entrance_;
    std::string exit_name_;
    Point3 exit_pos_;
    Door* exit_;
};

#endif // DOORTAB_H
