#ifndef TECH_GAME_STATE_PHASESTATE_H
#define TECH_GAME_STATE_PHASESTATE_H
#include <vector>

#include "Phase.h"
#include "memory_resource"


class PhaseState {
    std::chrono::time_point<std::chrono::steady_clock> start_phase_bot_a, start_phase_bot_b;
    std::vector<Phase> m_open_phases;
    Phase phase_bot_a;
    Phase phase_bot_b;

public:
    explicit PhaseState(Phase phase_bot_a, Phase phase_bot_b);

    [[nodiscard]] std::chrono::time_point<std::chrono::steady_clock> get_start_phase_bot_a() const {
        return start_phase_bot_a;
    }

    [[nodiscard]] std::chrono::time_point<std::chrono::steady_clock> get_start_phase_bot_b() const {
        return start_phase_bot_b;
    }

    [[nodiscard]] Phase get_phase_bot_a() const {
        return phase_bot_a;
    }

    [[nodiscard]] Phase get_phase_bot_b() const {
        return phase_bot_b;
    }

    [[nodiscard]] std::vector<Phase> &get_completed_phases() {
        return m_open_phases;
    }
};


#endif //TECH_GAME_STATE_PHASESTATE_H
