# GameState

Game State Library for ECER2026 TechSupport Paper

Game State (Problems):
**Phase**:

- Phase does not initialize completion conditions only conditions
- Allowed agents is vague
  **Phase State**:
- Time is not taken into consideration enough yet
    - timeout = estimated time
    - actual time is now - start
    - maybe add a buffer range in which it is still possible else count as impossible
    - open_phases do not get removed
      **Game Table State**:
- Change out std::any for template helpers
- Make logic more ironclad all primitive datatypes as well as dynamic arrays of primitive datatypes are allowed
  **General:**
- Naming is inconsistent
- Uncler how to use game state from bot_b
- Difficult to program for Vanja and Chrisi because they need to have the functions before using the game state plus
  there need to be functions from every phase on wards
- Did not think about how to transition between phases while taking into consideration that position will change