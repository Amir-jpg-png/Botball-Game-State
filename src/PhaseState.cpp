#include "PhaseState.h"

#include <utility>

using json = nlohmann::json;

PhaseState::PhaseState(Phase phase_bot_a, Phase phase_bot_b) : m_open_phases({}),
                                                               m_phase_bot_a(std::move(phase_bot_a)),
                                                               m_phase_bot_b(std::move(phase_bot_b)) {
    start_phase_bot_a = std::chrono::steady_clock::now();
    start_phase_bot_b = std::chrono::steady_clock::now();
}

void PhaseState::set_phase_bot_a(Phase phase_bot_a) {
    m_phase_bot_a = std::move(phase_bot_a);
}

void PhaseState::set_phase_bot_b(Phase phase_bot_b) {
    m_phase_bot_b = std::move(phase_bot_b);
}
