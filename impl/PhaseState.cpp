#include "PhaseState.h"
#include <utility>

using json = nlohmann::json;

PhaseState::PhaseState(Phase phase_bot_a, Phase phase_bot_b) : m_open_phases({}),
                                                               m_phase_id_a(std::move(phase_bot_a.get_id())),
                                                               m_phase_id_b(std::move(phase_bot_b.get_id())) {
}

void PhaseState::set_phase_id_a(const std::string& phase_id) {
    m_phase_id_a = phase_id;
}

void PhaseState::set_phase_id_b(const std::string& phase_id) {
    m_phase_id_b = phase_id;
}

std::string PhaseState::get_phase_id_a() {
    return m_phase_id_a;
}

std::string PhaseState::get_phase_id_b() {
    return m_phase_id_b;
}

std::vector<Phase> &PhaseState::get_open_phases() {
    return m_open_phases;
}

void PhaseState::remove_phase(const std::string &phase_id) {
    std::erase_if(m_open_phases,
                  [&](const Phase &p) { return p.get_id() == phase_id; });
}

const std::vector<Phase> &PhaseState::get_open_phases_const() const {
    return m_open_phases;
}

Phase PhaseState::get_phase(const std::string &phase_id) {
    for (Phase &phase: m_open_phases) {
        if (phase.get_id() == phase_id) {
            return phase;
        }
    }
    fatal("failed to find phase with id " + phase_id, m_log);
}

bool PhaseState::has_phase(const std::string &phase_id) {
    for (Phase &phase: m_open_phases) {
        if (phase.get_id() == phase_id) {
            return true;
        }
    }
    return false;
}

Phase *PhaseState::get_phase_ptr(const std::string &phase_id) {
    for (Phase &phase: m_open_phases) {
        if (phase.get_id() == phase_id) {
            return &phase;
        }
    }
    fatal("failed to find phase with id " + phase_id, m_log);
}

void PhaseState::clean() {
    auto& phases = get_open_phases();
    std::erase_if(phases,
                  [](const Phase& p) {
                      return p.get_done() || p.get_status() == TIMEOUT;
                  });
}