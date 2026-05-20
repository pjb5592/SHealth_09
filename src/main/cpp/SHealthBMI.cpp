#include "SHealth.h"

#include "BmiDomain.h"

#include <cstdio>

int main() {
    SHealth shealth;
    shealth.calculateBmi("shealth.dat");

    for (const shealth::domain::AgeCohortDescriptor& cohort : shealth::domain::kAgeCohorts) {
        const auto& firstCategory = shealth::domain::kBmiCategories[0];
        printf("%d - %s = %f", cohort.ageClass, firstCategory.label,
               shealth.getBmiRatio(cohort.ageClass, firstCategory.apiType));
        for (std::size_t i = 1; i < shealth::domain::kBmiCategoryTableSize; ++i) {
            const auto& category = shealth::domain::kBmiCategories[i];
            printf(", %s = %f", category.label,
                   shealth.getBmiRatio(cohort.ageClass, category.apiType));
        }
        printf("\n");
    }

    return 0;
}
