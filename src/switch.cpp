#include "switch.h"

#include "roommap.h"
#include "delta.h"
#include "graphicsmanager.h"
#include "mapfile.h"

#include <algorithm>



Switch::Switch(Point3 pos, bool persistent, bool active): GameObject(pos),
persistent_ {persistent}, active_ {active}, signalers_ {} {}

Switch::~Switch() {}

void Switch::push_signaler(Signaler* signaler) {
    signalers_.push_back(signaler);
}

void Switch::toggle() {
    active_ = !active_;
    for (auto& signaler : signalers_) {
        signaler->receive_signal(active_);
    }
}
