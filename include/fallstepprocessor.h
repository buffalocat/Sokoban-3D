#ifndef FALLSTEPPROCESSOR_H
#define FALLSTEPPROCESSOR_H

#include <memory>
#include <unordered_set>

#include "component.h"

class RoomMap;
class DeltaFrame;
class SnakeBlock;

class FallStepProcessor {
public:
    FallStepProcessor(RoomMap*, DeltaFrame*, std::vector<GameObject*>&&);
    ~FallStepProcessor();

    bool run();
    void check_land_first(FallComponent* comp);
    void collect_above(FallComponent* comp, std::vector<GameObject*>& above_list);
    bool drop_check(FallComponent* comp);
    void check_land_sticky(FallComponent* comp);
    void handle_fallen_blocks(FallComponent* comp);
    void settle(FallComponent* comp);

private:
    std::vector<std::unique_ptr<FallComponent>> fall_comps_unique_;
    std::vector<GameObject*> fall_check_;
    std::unordered_set<SnakeBlock*> snake_check_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    int layers_fallen_;
};

#endif // FALLSTEPPROCESSOR_H
