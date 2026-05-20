#pragma once

#include <cstddef>

namespace shealth {
namespace domain {

enum class BmiCategoryType : int {
    Underweight = 100,
    Normal = 200,
    Overweight = 300,
    Obesity = 400,
};

enum class BmiCategoryIndex : int {
    Underweight = 0,
    Normal = 1,
    Overweight = 2,
    Obesity = 3,
};

constexpr int kAgeClassMin = 20;
constexpr int kAgeClassMax = 70;
constexpr int kAgeClassStep = 10;
constexpr int kAgeCohortCount = (kAgeClassMax - kAgeClassMin) / kAgeClassStep + 1;
constexpr int kBmiCategoryCount = 4;

constexpr double kBmiUnderweightMax = 18.5;
constexpr double kBmiNormalUpperExclusive = 23.0;
constexpr double kBmiOverweightMaxExclusive = 25.0;
constexpr double kCmPerMeter = 100.0;
constexpr double kPercentScale = 100.0;

struct AgeCohortDescriptor {
    int ageClass;
    int cohortIndex;
    int minAge;
    int maxAgeExclusive;
};

struct BmiCategoryDescriptor {
    BmiCategoryType apiType;
    BmiCategoryIndex storageIndex;
    const char* label;
};

inline constexpr AgeCohortDescriptor kAgeCohorts[] = {
    {20, 0, 20, 30},
    {30, 1, 30, 40},
    {40, 2, 40, 50},
    {50, 3, 50, 60},
    {60, 4, 60, 70},
    {70, 5, 70, 80},
};

inline constexpr BmiCategoryDescriptor kBmiCategories[] = {
    {BmiCategoryType::Underweight, BmiCategoryIndex::Underweight, "underweight"},
    {BmiCategoryType::Normal, BmiCategoryIndex::Normal, "normal"},
    {BmiCategoryType::Overweight, BmiCategoryIndex::Overweight, "overweight"},
    {BmiCategoryType::Obesity, BmiCategoryIndex::Obesity, "obesity"},
};

inline constexpr std::size_t kAgeCohortTableSize = sizeof(kAgeCohorts) / sizeof(kAgeCohorts[0]);
inline constexpr std::size_t kBmiCategoryTableSize =
    sizeof(kBmiCategories) / sizeof(kBmiCategories[0]);

inline bool isAgeInCohort(int age, const AgeCohortDescriptor& cohort) {
    return age >= cohort.minAge && age < cohort.maxAgeExclusive;
}

inline bool isInAgeCohort(int age, int ageClass) {
    for (const AgeCohortDescriptor& cohort : kAgeCohorts) {
        if (cohort.ageClass == ageClass) {
            return isAgeInCohort(age, cohort);
        }
    }
    return false;
}

inline int ageClassToCohortIndex(int ageClass) {
    for (const AgeCohortDescriptor& cohort : kAgeCohorts) {
        if (cohort.ageClass == ageClass) {
            return cohort.cohortIndex;
        }
    }
    return -1;
}

inline int typeToCategoryIndex(int type) {
    for (const BmiCategoryDescriptor& category : kBmiCategories) {
        if (static_cast<int>(category.apiType) == type) {
            return static_cast<int>(category.storageIndex);
        }
    }
    return -1;
}

inline int classifyBmiCategory(double bmi) {
    if (bmi <= kBmiUnderweightMax) {
        return static_cast<int>(BmiCategoryIndex::Underweight);
    }
    if (bmi < kBmiNormalUpperExclusive) {
        return static_cast<int>(BmiCategoryIndex::Normal);
    }
    if (bmi < kBmiOverweightMaxExclusive) {
        return static_cast<int>(BmiCategoryIndex::Overweight);
    }
    return static_cast<int>(BmiCategoryIndex::Obesity);
}

inline double computeBmi(double weightKg, double heightCm) {
    const double heightM = heightCm / kCmPerMeter;
    return weightKg / (heightM * heightM);
}

}  // namespace domain
}  // namespace shealth
