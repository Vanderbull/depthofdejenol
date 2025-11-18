#ifndef COMPLICATIONEFFECT_H
#define COMPLICATIONEFFECT_H

#include <string>
#include <map>

// Struct to hold the base character stats, making it easy to pass and modify them.
struct CharacterStats {
    int strength = 0;
    int dexterity = 0;
    int intelligence = 0;
    int charisma = 0;
    int constitution = 0;
    int wisdom = 0;
    
    int maxHits = 0;
    int ageInYears = 0;
};

// Map to hold the racial minimums for stats (you would populate this from game data)
using RacialMinimums = std::map<std::string, int>;

class ComplicationEffect {
public:
    /**
     * @brief Applies all the consequences of a complication to the character's stats.
     * @param currentStats The character's current stats, which will be modified.
     * @param minStats The minimum allowable racial stats.
     */
    void applyEffects(CharacterStats& currentStats, const CharacterStats& minStats);

    /**
     * @brief Calculates the reduction in Max Hits as a percentage.
     */
    static constexpr float getMaxHitsReductionFactor() {
        return 0.8333f; // 100% - 16.67%
    }
    
    /**
     * @brief Returns the aging effect in years.
     */
    static constexpr int getAgeIncrease() {
        return 15;
    }

private:
    /**
     * @brief Reduces a single stat by 5, ensuring it does not go below the racial minimum.
     * @param stat The current stat value (will be modified).
     * @param minimum The racial minimum for that stat.
     */
    void reduceStat(int& stat, int minimum);
};

#endif // COMPLICATIONEFFECT_H
