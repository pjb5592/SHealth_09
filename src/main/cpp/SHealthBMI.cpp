#include "SHealth.h"

#include <cstdio>

int main() {
    SHealth shealth;
    shealth.calculateBmi("shealth.dat");

    for (int ageClass = 20; ageClass <= 70; ageClass += 10) {
        printf("%d - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
               ageClass,
               shealth.getBmiRatio(ageClass,
                                   static_cast<int>(SHealth::BmiCategoryType::Underweight)),
               shealth.getBmiRatio(ageClass, static_cast<int>(SHealth::BmiCategoryType::Normal)),
               shealth.getBmiRatio(ageClass,
                                   static_cast<int>(SHealth::BmiCategoryType::Overweight)),
               shealth.getBmiRatio(ageClass, static_cast<int>(SHealth::BmiCategoryType::Obesity)));
    }

    return 0;
}
