#include "PhaseState.h"
#include <utility>

using json = nlohmann::json;

PhaseState::PhaseState(Phase phase_bot_a, Phase phase_bot_b) : m_open_phases({}),
                                                               m_phase_bot_a(std::move(phase_bot_a)),
                                                               m_phase_bot_b(std::move(phase_bot_b)) {
}

void PhaseState::set_phase_bot_a(Phase phase_bot_a) {
    m_phase_bot_a = std::move(phase_bot_a);
}

void PhaseState::set_phase_bot_b(Phase phase_bot_b) {
    m_phase_bot_b = std::move(phase_bot_b);
}

Phase &PhaseState::get_phase_bot_a() {
    return m_phase_bot_a;
}

Phase &PhaseState::get_phase_bot_b() {
    return m_phase_bot_b;
}

std::vector<Phase> &PhaseState::get_open_phases() {
    return m_open_phases;
}

void PhaseState::remove_phase(const std::string &key) {
    for (size_t i = 0; i < m_open_phases.size(); i++) {
        if (m_open_phases[i].get_id() == key) {
            m_open_phases.erase(m_open_phases.begin() + i);
        }
    }
}

const std::vector<Phase> &PhaseState::get_open_phases_const() const {
    return m_open_phases;
}
