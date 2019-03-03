#ifndef SWITCHTAB_H
#define SWITCHTAB_H

#include "common.h"
#include "editortab.h"

class Switch;
class Switchable;
class Signaler;

class SwitchTab: public EditorTab {
public:
    SwitchTab(EditorState*, GraphicsManager*);
    ~SwitchTab();

    void init();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);
    void handle_right_click(EditorRoom*, Point3);

    int get_signaler_labels(const char* labels[], std::vector<std::unique_ptr<Signaler>>& signalers);

private:
    std::vector<Switch*> model_switches_;
    std::vector<Switchable*> model_switchables_;
};

#endif // SWITCHTAB_H
