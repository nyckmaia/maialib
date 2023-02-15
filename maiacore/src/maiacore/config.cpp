#include "maiacore/config.h"

TuningSystem _tuningSystem = TuningSystem::EQUAL_TEMPERAMENT;

TuningSystem getTuningSystem() { return _tuningSystem; }
void setTuningSystem(TuningSystem tuningSystem) { _tuningSystem = tuningSystem; }