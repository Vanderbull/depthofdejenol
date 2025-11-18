#ifndef COMPLICATIONCALCULATOR_H
#define COMPLICATIONCALCULATOR_H

/**
 * @brief Calculates the percentage chance of complications using the formula:
 * (AgeInYears * 2 / RaceMaxAge) * 100
 * @param ageInYears The age of the individual in years.
 * @param raceMaxAge The maximum expected age for the individual's race/group. Must be > 0.
 * @return float The calculated chance of complications as a percentage (clamped at 100.0f).
 */
float calculateComplicationChance(int ageInYears, int raceMaxAge);

#endif // COMPLICATIONCALCULATOR_H
