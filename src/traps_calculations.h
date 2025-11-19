#pragma once
#include <cmath>
#include <algorithm>

// Clamp utility for percentage limits
inline double clamp(double value, double minval, double maxval) {
    return std::max(minval, std::min(value, maxval));
}

// 1. Chance to disarm a trap
// Returns a percentage (double between 5 and 98)
inline double disarmTrapChance(int DEX, int WIS, int gLvl, double thiefMod, int mLvl, int dLvl)
{
    double part1 = DEX / 2.0 + WIS / 4.0;
    double part2 = 4.5 + std::exp(std::log(gLvl + 1) / std::log(15.0));
    double part3 = thiefMod * 1.4;
    double part4 = std::log(mLvl) * std::log(dLvl + 1) * 9.375;
    double result = part1 + part2 * part3 - part4;
    // Clamp result to min = 5%, max = 98%
    return clamp(result, 5.0, 98.0);
}

// 2. Chance to identify a trap
// Returns a percentage (double between 2 and 98)
inline double identifyTrapChance(int WIS, int mLvl, int mDex, int gLvl, double uncappedDisarmChance)
{
    double base = std::floor(WIS / 4.0) - std::floor(std::log(mLvl) * std::log(mDex) * 2.5);
    double percent = std::floor(std::floor(base + (uncappedDisarmChance / 1.2)) + std::pow(std::log(gLvl), 2));
    // Clamp result to min = 2%, max = 98%
    return clamp(percent, 2.0, 98.0);
}

// 3. Thieving ability (cosmetic)
// Returns a percentage between 0 and 100 (double)
inline double thievingAbility(double X, double Y, double Z)
{
    if(Z == Y) return 0.0; // prevent division by zero
    double ability = std::floor((X - Y) / (Z - Y) * 100.0);
    return clamp(ability, 0.0, 100.0); // normalized to 0-100
}

// Helper to calculate X/Y/Z for thieving ability (for parameters as in doc)
// X = [DEX/2 + WIS/2] + [4.5 + exp(ln(gLvl + 1) / ln(15))] * [thiefMod * 1.4] - [ln(2) * ln(2) * 9.375]
inline double thievingParam(int DEX, int WIS, int gLvl, double thiefMod)
{
    double part1 = DEX / 2.0 + WIS / 2.0;
    double part2 = 4.5 + std::exp(std::log(gLvl + 1) / std::log(15.0));
    double part3 = thiefMod * 1.4;
    double part4 = std::log(2.0) * std::log(2.0) * 9.375;
    return part1 + part2 * part3 - part4;
}
