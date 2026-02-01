#ifndef TECH_GAME_STATE_PHASESTATE_H
#define TECH_GAME_STATE_PHASESTATE_H
#include <vector>

#include "Phase.h"
#include "memory_resource"


class PhaseState {
    std::chrono::time_point<std::chrono::steady_clock> start_phase_bot_a, start_phase_bot_b;
    std::vector<Phase> completed_phases;
    Phase phase_bot_a;
    Phase phase_bot_b;
public:
    explicit PhaseState(Phase phase_bot_a, Phase phase_bot_b);
};


#endif //TECH_GAME_STATE_PHASESTATE_H