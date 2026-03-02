#ifndef TECH_GAME_STATE_PHASE_H
#define TECH_GAME_STATE_PHASE_H
#include <string>
#include "TableState.h"

/**
 * Shows the status of a phase\n\n\n
 * OPEN -> Not done yet and schedulable\n\n
 * BLOCKED -> Conditions not met (can transition to OPEN or TIMEOUT)\n\n
 * TIMEOUT -> Not enough time left to complete the phase\n\n
 * DONE -> Phase completed successfully\n\n
 */
enum PhaseStatus {
    OPEN,
    BLOCKED,
    TIMEOUT,
    DONE
};

/**
 * Takes a PhaseStatus (which is an int) and converts it to a string to support the user in debugging
 * @param status to be converted to a string
 * @return a string representing one of four PhaseStatus options
 */
inline std::string statusCodeToString(const PhaseStatus status) {
    switch (status) {
        case OPEN:
            return "OPEN";
        case BLOCKED:
            return "BLOCKED";
        case TIMEOUT:
            return "TIMEOUT";
        case DONE:
            return "DONE";
        default:
            throw std::logic_error("Unknown phase status");
    }
}

/**
 * An Object representing a task on the Botball Game Table
 */
class Phase {
    int m_time_to_completion;
    int m_points;
    PhaseStatus m_status;
    std::string m_id;
    // Determines which bot "bot_a" or "bot_b" is allowed to execute this phase
    std::string m_allowed_agent;

    // key value pairs of strings to primitive datatypes representing if the phases conditions are met
    std::unordered_map<std::string, std::any> m_conditions;
    // key value pairs of strings to primitive datatypes which will be set in the table state after completion of the phase
    std::unordered_map<std::string, std::any> m_completion;

public:
    explicit Phase(const std::string &phase_id, const json &data);

    /**
     * @return the phase id
     */
    [[nodiscard]] const std::string get_id() const;

    /**
     * @return the amount of time it takes to complete the phase as an int
     */
    [[nodiscard]] int get_time_to_completion() const;

    /**
     * @return the agent ("bot_a" or "bot_b") that may complete the phase
     */
    [[nodiscard]] const std::string &get_allowed_agent() const;

    /**
     *
     * @return an unordered map of strings -> primitive dataypes representing the conditions that have to be met before the phase may be executed
     */
    [[nodiscard]] const std::unordered_map<std::string, std::any> &get_conditions() const;

    /**
     * @return an unordered map of strings -> primitive datatypes representing the conditions that will be set in the table state after the successful execution of the phase
     */
    [[nodiscard]] const std::unordered_map<std::string, std::any> &get_completion() const;

    /**
     * @return true if the phase was executed
     */
    [[nodiscard]] bool get_done() const;

    /**
     *
     * @return the amount of points to be earned by the phase
     */
    [[nodiscard]] int get_points() const;

    /**
     *
     * @return the PhaseStatus of a phase)
     */
    [[nodiscard]] PhaseStatus get_status() const;

    /**
     * Sets the PhaseStatus of a phase
     * @param status the phase should be in
     */
    void set_status(PhaseStatus status);

    /**
     * Sets the PhaseStatus of a phase and sends it to the peer bot
     * @param status the phase should be in
     * @param socket used to send a request to the peer bot
     */
    void set_status(PhaseStatus status, const Socket &so);

    /**
     * Executes the phase action, mutates the table state to reflect the changes made by the action and sets the PhaseStatus to DONE
     * @param table a reference to the table state so that it may be changed on commpletion
     * @param action a void(void) function that will be executed by the phase
     * @param so socket to pass to table state for sending updates over the network
     */
    void execute(TableState &table, const std::function<void()> &action, const Socket &so);

    std::string to_string() const;
};

#endif // TECH_GAME_STATE_PHASE_H
