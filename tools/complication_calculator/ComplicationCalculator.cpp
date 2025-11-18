#include "ComplicationCalculator.h"

float calculateComplicationChance(int ageInYears, int raceMaxAge) {
    // Check for invalid input to prevent division by zero
    if (raceMaxAge <= 0) {
        return 0.0f; 
    }

    // Cast to float for accurate division and percentage calculation
    float chance = (static_cast<float>(ageInYears) * 2.0f / static_cast<float>(raceMaxAge)) * 100.0f;

    // Clamp the result to a maximum of 100%
    if (chance > 100.0f) {
        return 100.0f;
    }
    
    return chance;
}
