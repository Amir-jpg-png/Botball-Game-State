#include "PhaseState.h"

#include <utility>

using json = nlohmann::json;

PhaseState::PhaseState(Phase phase_bot_a, Phase phase_bot_b):
      completed_phases({}),
      phase_bot_a(std::move(phase_bot_a)),
      phase_bot_b(std::move(phase_bot_b))
{
}
