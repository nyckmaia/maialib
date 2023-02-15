enum class TuningSystem {
    EQUAL_TEMPERAMENT,
    JUST_INTONATION,
    PYTHAGOREAN_TUNING,
    MEANTONE_TEMPERAMENT,
    WELL_TEMPERAMENT
};

TuningSystem getTuningSystem();
void setTuningSystem(TuningSystem tuningSystem);