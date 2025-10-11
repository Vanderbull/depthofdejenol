#include "ComplicationEffect.h"
#include <algorithm> // For std::max or std::min if needed

// --- Private Helper Function ---

void ComplicationEffect::reduceStat(int& stat, int minimum) {
    // Stat is reduced by 5, but not below the racial minimum.
    stat = std::max(minimum, stat - 5);
}

// --- Public Effect Application Function ---

void ComplicationEffect::applyEffects(CharacterStats& currentStats, const CharacterStats& minStats) {
    // 1. Stat Reductions (-5 to each, not below racial minimums)
    reduceStat(currentStats.strength, minStats.strength);
    reduceStat(currentStats.dexterity, minStats.dexterity);
    reduceStat(currentStats.intelligence, minStats.intelligence);
    reduceStat(currentStats.charisma, minStats.charisma);
    reduceStat(currentStats.constitution, minStats.constitution);
    reduceStat(currentStats.wisdom, minStats.wisdom);

    // 2. Max Hits Reduction (reduced by 16.67%, i.e., multiplied by 0.8333)
    // Using float for calculation and casting back to int for the final result
    float newMaxHits = static_cast<float>(currentStats.maxHits) * getMaxHitsReductionFactor();
    currentStats.maxHits = static_cast<int>(newMaxHits);

    // 3. Age Increase (+15 years)
    currentStats.ageInYears += getAgeIncrease();

    // NOTE: A/D changes are implicitly handled by the stat reductions above 
    // when the calling code updates the character's derived stats and equipment.
}
