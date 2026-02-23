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
    std::erase_if(m_open_phases,
                  [&](const Phase &p) { return p.get_id() == key; });
}

const std::vector<Phase> &PhaseState::get_open_phases_const() const {
    return m_open_phases;
}

Phase &PhaseState::get_phase(const std::string &phase_id) {
    for (Phase &phase: m_open_phases) {
        if (phase.get_id() == phase_id) {
            return phase;
        }
    }
    fatal("failed to find phase with id " + phase_id, m_log);
}

Phase *PhaseState::get_phase_ptr(const std::string &phase_id) {
    for (Phase &phase: m_open_phases) {
        if (phase.get_id() == phase_id) {
            return &phase;
        }
    }
    fatal("failed to find phase with id " + phase_id, m_log);
}
