#ifndef TECH_GAME_STATE_PHASE_STATE_H
#define TECH_GAME_STATE_PHASE_STATE_H
#include <vector>
#include "Phase.h"

class PhaseState {
    std::vector<Phase> m_open_phases;
    Phase m_phase_bot_a;
    Phase m_phase_bot_b;
    std::shared_ptr<spdlog::logger> m_log = create_logger("PS");

public:
    explicit PhaseState(Phase phase_bot_a, Phase phase_bot_b);

    /**
     * Returns a reference to the phase that is currently running on bot_a, this reference is mutable
     * @return a mutable reference to the phase running on bot_b
     */
    [[nodiscard]] Phase &get_phase_bot_a();

    /**
     * Returns a reference to the phase that is currently running on bot_b, this reference is mutable
     * @return a mutable reference to the phase running on bot_a
     */
    [[nodiscard]] Phase &get_phase_bot_b();

    /**
     * Sets the phase that bot_a is executing
     * @param phase_bot_a the phase that bot_a is to execute
     */
    void set_phase_bot_a(Phase phase_bot_a);

    /**
     * Sets the phase that bot_b is executing
     * @param phase_bot_b the phase that bot_b is to execute
     */
    void set_phase_bot_b(Phase phase_bot_b);

    /**
     * Returns a reference to a vector of all phases that are not DONE, this reference is mutable
     * @return a reference to a vector of all open phases on both bots
     */
    [[nodiscard]] std::vector<Phase> &get_open_phases();

    const std::vector<Phase> &get_open_phases_const() const;

    void remove_phase(const std::string &key);

    Phase &get_phase(const std::string &phase_id);

    Phase *get_phase_ptr(const std::string &phase_id);
};


#endif //TECH_GAME_STATE_PHASE_STATE_H
