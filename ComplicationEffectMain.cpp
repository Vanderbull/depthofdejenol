#include <iostream>
#include "ComplicationEffect.h"

int main() {
    // --- Initial Character State ---
    CharacterStats heroStats = {
        .strength = 60,
        .dexterity = 55,
        .intelligence = 40,
        .charisma = 35,
        .constitution = 50,
        .wisdom = 45,
        .maxHits = 500,
        .ageInYears = 45
    };
    
    // --- Racial Minimums ---
    CharacterStats racialMins = {
        .strength = 15,
        .dexterity = 15,
        .intelligence = 15,
        .charisma = 15,
        .constitution = 15,
        .wisdom = 15,
        .maxHits = 0, // Not applicable for mins
        .ageInYears = 0 // Not applicable for mins
    };

    std::cout << "--- BEFORE Complications ---\n";
    std::cout << "Strength: " << heroStats.strength 
              << ", Max Hits: " << heroStats.maxHits
              << ", Age: " << heroStats.ageInYears << "\n\n";

    // --- Apply the Effect ---
    ComplicationEffect effect;
    effect.applyEffects(heroStats, racialMins);

    std::cout << "--- AFTER Complications ---\n";
    std::cout << "Strength: " << heroStats.strength      // Should be 55
              << ", Dexterity: " << heroStats.dexterity  // Should be 50
              << ", Constitution: " << heroStats.constitution // Should be 45
              << "\n";
    std::cout << "Max Hits: " << heroStats.maxHits        // Should be 416 (500 * 0.8333)
              << ", Age: " << heroStats.ageInYears << "\n"; // Should be 60

    // Demonstrate Stat Floor Protection
    heroStats.strength = 18;
    effect.applyEffects(heroStats, racialMins);
    std::cout << "\nAfter second complication (Strength reduced from 18):\n";
    std::cout << "Strength: " << heroStats.strength << " (Min is 15)\n"; // Should be 15
    
    return 0;
}
