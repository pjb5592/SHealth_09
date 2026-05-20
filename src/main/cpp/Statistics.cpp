#include "Statistics.h"

namespace shealth {
namespace detail {
namespace stats {

namespace domain = shealth::domain;

void computeAgeCohortRatios(const std::vector<PersonRecord>& records,
                            CohortRatios& cohortRatios) {
    const std::size_t count = records.size();
    for (const domain::AgeCohortDescriptor& cohort : domain::kAgeCohorts) {
        int categoryCounts[domain::kBmiCategoryCount] = {0, 0, 0, 0};
        int sum = 0;
        for (std::size_t i = 0; i < count; i++) {
            if (!domain::isAgeInCohort(records[i].age, cohort)) {
                continue;
            }
            sum++;
            const int category = domain::classifyBmiCategory(records[i].bmi);
            if (category >= 0) {
                categoryCounts[category]++;
            }
        }
        const int cohortIndex = cohort.cohortIndex;
        if (sum == 0) {
            for (const domain::BmiCategoryDescriptor& category : domain::kBmiCategories) {
                cohortRatios[cohortIndex][static_cast<int>(category.storageIndex)] = 0.0;
            }
            continue;
        }
        for (const domain::BmiCategoryDescriptor& category : domain::kBmiCategories) {
            const int storageIndex = static_cast<int>(category.storageIndex);
            cohortRatios[cohortIndex][storageIndex] =
                static_cast<double>(categoryCounts[storageIndex]) * domain::kPercentScale / sum;
        }
    }
}

void computeOverallRatios(const std::vector<PersonRecord>& records,
                          OverallRatios& overallRatios) {
    int categoryCounts[domain::kBmiCategoryCount] = {0, 0, 0, 0};
    const std::size_t count = records.size();
    if (count == 0) {
        overallRatios.fill(0.0);
        return;
    }
    for (std::size_t i = 0; i < count; i++) {
        const int category = domain::classifyBmiCategory(records[i].bmi);
        if (category >= 0) {
            categoryCounts[category]++;
        }
    }
    for (const domain::BmiCategoryDescriptor& category : domain::kBmiCategories) {
        const int storageIndex = static_cast<int>(category.storageIndex);
        overallRatios[storageIndex] = static_cast<double>(categoryCounts[storageIndex]) *
                                        domain::kPercentScale / static_cast<double>(count);
    }
}

}  // namespace stats
}  // namespace detail
}  // namespace shealth
