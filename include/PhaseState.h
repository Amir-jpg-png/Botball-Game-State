#pragma once
#include <vector>
#include "Phase.h"

class PhaseState {
    std::vector<Phase> m_open_phases;
    std::string m_phase_id_a;
    std::string m_phase_id_b;
    std::shared_ptr<spdlog::logger> m_log = create_logger("PS");

public:
    explicit PhaseState();

    /**
     * Gets the id of bot_a's currently active phase
     * @return the id of the phase running on bot_a
     */
    [[nodiscard]] std::string get_phase_id_a();

    /**
     * Gets the id of bot_b's currently active phase
     * @return the id of the phase running on bot_b
     */
    [[nodiscard]] std::string get_phase_id_b();

    /**
     * Sets the phase that bot_a is executing
     * @param phase_id_a the phase id of bot_a's next phase
     */
    void set_phase_id_a(const std::string &phase_id_a);

    /**
     * Sets the phase that bot_a is executing
     * @param phase_id_b the phase id of bot_b's next phase
     */
    void set_phase_id_b(const std::string &phase_id_b);

    /**
     * Returns a mutable reference to a vector of all phases that are not DONE or TIMEOUT
     * @return a mutable reference to a vector of all open phases on both bots
     */
    [[nodiscard]] std::vector<Phase> &get_open_phases();

    /**
     * Returns an immutable reference to a vector of all phases that are not DONE or TIMEOUT
     * @return an immutable reference to a vector of all open phases on both bots
     */
    [[nodiscard]] const std::vector<Phase> &get_open_phases_const() const;

    Phase get_phase(const std::string &phase_id);

    Phase *get_phase_ptr(const std::string &phase_id);

    /**
     * Determines whether a phase exists in all open phases
     * @param phase_id id of the phase to check
     * @return a bool representing if the phase exist or not
     */
    bool has_phase(const std::string &phase_id);

    /**
     * Iterates over all open phases and removes the phases that have status DONE or TIMEOUT
     */
    void clean();
};
