#include "SHealth.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace shealth {
namespace detail {

namespace bmi {

constexpr int kAgeClassMin = 20;
constexpr int kAgeClassMax = 70;
constexpr int kAgeClassStep = 10;
constexpr int kAgeCohortCount = 6;
constexpr int kBmiCategoryCount = 4;
constexpr double kCmPerMeter = 100.0;
constexpr double kBmiUnderweightMax = 18.5;
constexpr double kBmiNormalUpperExclusive = 23.0;
constexpr double kBmiOverweightMaxExclusive = 25.0;

constexpr int kTypeUnderweight = 100;
constexpr int kTypeNormal = 200;
constexpr int kTypeOverweight = 300;
constexpr int kTypeObesity = 400;

constexpr int kIdxUnderweight = 0;
constexpr int kIdxNormal = 1;
constexpr int kIdxOverweight = 2;
constexpr int kIdxObesity = 3;

bool isInAgeCohort(int age, int ageClass) {
    return age >= ageClass && age < ageClass + kAgeClassStep;
}

int ageClassToCohortIndex(int ageClass) {
    if (ageClass < kAgeClassMin || ageClass > kAgeClassMax ||
        (ageClass - kAgeClassMin) % kAgeClassStep != 0) {
        return -1;
    }
    return (ageClass - kAgeClassMin) / kAgeClassStep;
}

int typeToCategoryIndex(int type) {
    switch (type) {
        case kTypeUnderweight:
            return kIdxUnderweight;
        case kTypeNormal:
            return kIdxNormal;
        case kTypeOverweight:
            return kIdxOverweight;
        case kTypeObesity:
            return kIdxObesity;
        default:
            return -1;
    }
}

int classifyBmiCategory(double bmi) {
    if (bmi <= kBmiUnderweightMax) {
        return kIdxUnderweight;
    }
    if (bmi < kBmiNormalUpperExclusive) {
        return kIdxNormal;
    }
    if (bmi < kBmiOverweightMaxExclusive) {
        return kIdxOverweight;
    }
    return kIdxObesity;
}

double computeBmi(double weightKg, double heightCm) {
    const double heightM = heightCm / kCmPerMeter;
    return weightKg / (heightM * heightM);
}

}  // namespace bmi

namespace csv {

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool loadFromCsv(const std::string& filename, int& count, int* ids, int* ages, double* weights,
                 double* heights, int maxRecords) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        const std::vector<std::string> tokens = split(line, ',');
        if (tokens.empty()) {
            continue;
        }
        if (count >= maxRecords) {
            break;
        }
        if (tokens.size() < 4) {
            return false;
        }
        try {
            ids[count] = std::stoi(tokens[0]);
            ages[count] = std::stoi(tokens[1]);
            weights[count] = std::stod(tokens[2]);
            heights[count] = std::stod(tokens[3]);
        } catch (const std::exception&) {
            return false;
        }
        count++;
    }
    return true;
}

}  // namespace csv

namespace impute {

void fillCohortAverage(int count, const int* ages, double* values, bool (*isMissing)(double)) {
    for (int ageClass = bmi::kAgeClassMin; ageClass <= bmi::kAgeClassMax;
         ageClass += bmi::kAgeClassStep) {
        double sum = 0;
        int validCount = 0;
        for (int i = 0; i < count; i++) {
            if (!bmi::isInAgeCohort(ages[i], ageClass)) {
                continue;
            }
            if (isMissing(values[i])) {
                continue;
            }
            sum += values[i];
            validCount++;
        }
        if (validCount == 0) {
            continue;
        }
        const double average = sum / validCount;
        for (int i = 0; i < count; i++) {
            if (!bmi::isInAgeCohort(ages[i], ageClass)) {
                continue;
            }
            if (isMissing(values[i])) {
                values[i] = average;
            }
        }
    }
}

bool isZero(double value) { return value == 0.0; }

void fillWeightZeros(int count, const int* ages, double* weights) {
    fillCohortAverage(count, ages, weights, isZero);
}

void fillHeightZeros(int count, const int* ages, double* heights) {
    fillCohortAverage(count, ages, heights, isZero);
}

}  // namespace impute

namespace stats {

constexpr double kPercentScale = 100.0;

void computeAgeCohortRatios(
    int count, const int* ages, const double* bmis,
    std::array<std::array<double, bmi::kBmiCategoryCount>, bmi::kAgeCohortCount>& cohortRatios) {
    for (int ageClass = bmi::kAgeClassMin; ageClass <= bmi::kAgeClassMax;
         ageClass += bmi::kAgeClassStep) {
        int categoryCounts[bmi::kBmiCategoryCount] = {0, 0, 0, 0};
        int sum = 0;
        for (int i = 0; i < count; i++) {
            if (!bmi::isInAgeCohort(ages[i], ageClass)) {
                continue;
            }
            sum++;
            const int category = bmi::classifyBmiCategory(bmis[i]);
            if (category >= 0) {
                categoryCounts[category]++;
            }
        }
        const int cohortIndex = bmi::ageClassToCohortIndex(ageClass);
        if (sum == 0) {
            for (int category = 0; category < bmi::kBmiCategoryCount; category++) {
                cohortRatios[cohortIndex][category] = 0.0;
            }
            continue;
        }
        for (int category = 0; category < bmi::kBmiCategoryCount; category++) {
            cohortRatios[cohortIndex][category] =
                static_cast<double>(categoryCounts[category]) * kPercentScale / sum;
        }
    }
}

void computeOverallRatios(int count, const double* bmis,
                          std::array<double, bmi::kBmiCategoryCount>& overallRatios) {
    int categoryCounts[bmi::kBmiCategoryCount] = {0, 0, 0, 0};
    if (count == 0) {
        overallRatios.fill(0.0);
        return;
    }
    for (int i = 0; i < count; i++) {
        const int category = bmi::classifyBmiCategory(bmis[i]);
        if (category >= 0) {
            categoryCounts[category]++;
        }
    }
    for (int category = 0; category < bmi::kBmiCategoryCount; category++) {
        overallRatios[category] =
            static_cast<double>(categoryCounts[category]) * kPercentScale / count;
    }
}

}  // namespace stats

}  // namespace detail
}  // namespace shealth

bool SHealth::isInAgeCohort(int age, int ageClass) {
    return shealth::detail::bmi::isInAgeCohort(age, ageClass);
}

int SHealth::ageClassToCohortIndex(int ageClass) {
    return shealth::detail::bmi::ageClassToCohortIndex(ageClass);
}

int SHealth::typeToCategoryIndex(int type) {
    return shealth::detail::bmi::typeToCategoryIndex(type);
}

int SHealth::classifyBmiCategory(double bmi) {
    return shealth::detail::bmi::classifyBmiCategory(bmi);
}

double SHealth::computeBmi(double weightKg, double heightCm) {
    return shealth::detail::bmi::computeBmi(weightKg, heightCm);
}

bool SHealth::loadFromCsv(const std::string& filename) {
    return shealth::detail::csv::loadFromCsv(filename, count, ids, ages, weights, heights,
                                             kMaxRecords);
}

void SHealth::imputeMissingWeights() {
    shealth::detail::impute::fillWeightZeros(count, ages, weights);
}

void SHealth::imputeMissingHeights() {
    shealth::detail::impute::fillHeightZeros(count, ages, heights);
}

void SHealth::computeAllBmi() {
    for (int i = 0; i < count; i++) {
        bmis[i] = computeBmi(weights[i], heights[i]);
    }
}

void SHealth::computeAgeCohortRatios() {
    shealth::detail::stats::computeAgeCohortRatios(count, ages, bmis, cohortRatios_);
}

void SHealth::computeOverallRatios() {
    shealth::detail::stats::computeOverallRatios(count, bmis, overallRatios_);
}

int SHealth::calculateBmi(const std::string& filename) {
    count = 0;
    for (auto& cohort : cohortRatios_) {
        cohort.fill(0.0);
    }
    overallRatios_.fill(0.0);
    if (!loadFromCsv(filename)) {
        return 0;
    }

    imputeMissingWeights();
    imputeMissingHeights();
    computeAllBmi();
    computeAgeCohortRatios();
    computeOverallRatios();
    return count;
}

double SHealth::getBmiRatio(int ageClass, int type) const {
    const int cohortIndex = ageClassToCohortIndex(ageClass);
    const int categoryIndex = typeToCategoryIndex(type);
    if (cohortIndex < 0 || categoryIndex < 0) {
        return 0.0;
    }
    return cohortRatios_[cohortIndex][categoryIndex];
}

double SHealth::getOverallBmiRatio(int type) const {
    const int categoryIndex = typeToCategoryIndex(type);
    if (categoryIndex < 0 || count == 0) {
        return 0.0;
    }
    return overallRatios_[categoryIndex];
}

std::vector<int> SHealth::getNormalBmiUserIds() const {
    std::vector<int> normalIds;
    if (count == 0) {
        return normalIds;
    }
    const int normalIndex = static_cast<int>(BmiCategoryIndex::Normal);
    for (int i = 0; i < count; i++) {
        if (classifyBmiCategory(bmis[i]) == normalIndex) {
            normalIds.push_back(ids[i]);
        }
    }
    return normalIds;
}

std::vector<std::string> SHealth::split(const std::string& line, char delimiter) {
    return shealth::detail::csv::split(line, delimiter);
}
