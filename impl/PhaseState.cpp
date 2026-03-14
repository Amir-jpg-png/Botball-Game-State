#include "PhaseState.h"
#include <utility>

using json = nlohmann::json;

PhaseState::PhaseState() : m_open_phases({}), m_phase_id_a("INIT_A"),m_phase_id_b("INIT_B") {
}

void PhaseState::set_phase_id_a(const std::string &phase_id) {
    m_phase_id_a = phase_id;
}

void PhaseState::set_phase_id_b(const std::string &phase_id) {
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

}
