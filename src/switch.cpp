#include "switch.h"

#include "roommap.h"
#include "delta.h"
#include "graphicsmanager.h"
#include "mapfile.h"
#include "signaler.h"

#include <algorithm>



Switch::Switch(bool persistent, bool active):
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
