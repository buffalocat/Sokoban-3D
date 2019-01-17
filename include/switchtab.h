#ifndef SWITCHTAB_H
#define SWITCHTAB_H

#include "common.h"
#include "editortab.h"

class Switch;
class Switchable;

class SwitchTab: public EditorTab {
public:
    SwitchTab(EditorState*, GraphicsManager*);
    virtual ~SwitchTab();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);
    void handle_right_click(EditorRoom*, Point3);

private:
    std::vector<Switchable*> switchables_;
    std::vector<Switch*> switches_;
};

#endif // SWITCHTAB_H
