// main.cpp or any other file

#include <iostream>
#include <iomanip>
#include "ComplicationCalculator.h" // Include the header file

int main() {
    int age = 50;
    int maxAge = 90;

    // Call the function defined in your library
    float complicationRisk = calculateComplicationChance(age, maxAge);
    
    std::cout << "Age: " << age << ", Max Age: " << maxAge << std::endl;
    std::cout << "Calculated Risk: " << std::fixed 
              << std::setprecision(2) << complicationRisk << "%" << std::endl;
              
    return 0;
}
