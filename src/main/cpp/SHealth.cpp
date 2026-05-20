#include "SHealth.h"

#include <fstream>
#include <iostream>
#include <sstream>

bool SHealth::isInAgeCohort(int age, int ageClass) {
    return age >= ageClass && age < ageClass + kAgeClassStep;
}

int SHealth::ageClassToCohortIndex(int ageClass) {
    if (ageClass < kAgeClassMin || ageClass > kAgeClassMax ||
        (ageClass - kAgeClassMin) % kAgeClassStep != 0) {
        return -1;
    }
    return (ageClass - kAgeClassMin) / kAgeClassStep;
}

int SHealth::typeToCategoryIndex(int type) {
    switch (type) {
        case static_cast<int>(BmiCategoryType::Underweight):
            return static_cast<int>(BmiCategoryIndex::Underweight);
        case static_cast<int>(BmiCategoryType::Normal):
            return static_cast<int>(BmiCategoryIndex::Normal);
        case static_cast<int>(BmiCategoryType::Overweight):
            return static_cast<int>(BmiCategoryIndex::Overweight);
        case static_cast<int>(BmiCategoryType::Obesity):
            return static_cast<int>(BmiCategoryIndex::Obesity);
        default:
            return -1;
    }
}

int SHealth::classifyBmiCategory(double bmi) {
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

double SHealth::computeBmi(double weightKg, double heightCm) {
    const double heightM = heightCm / kCmPerMeter;
    return weightKg / (heightM * heightM);
}

bool SHealth::loadFromCsv(const std::string& filename) {
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
            break;
        }
        ages[count] = std::stoi(tokens[1]);
        weights[count] = std::stod(tokens[2]);
        heights[count] = std::stod(tokens[3]);
        count++;
    }
    return true;
}

void SHealth::imputeMissingWeights() {
    for (int ageClass = kAgeClassMin; ageClass <= kAgeClassMax; ageClass += kAgeClassStep) {
        double sum = 0;
        int ageCount = 0;
        for (int i = 0; i < count; i++) {
            if (!isInAgeCohort(ages[i], ageClass)) {
                continue;
            }
            if (weights[i] == 0.0) {
                continue;
            }
            sum += weights[i];
            ageCount++;
        }
        if (ageCount == 0) {
            continue;
        }
        for (int i = 0; i < count; i++) {
            if (!isInAgeCohort(ages[i], ageClass)) {
                continue;
            }
            if (weights[i] == 0.0) {
                weights[i] = sum / ageCount;
            }
        }
    }
}

void SHealth::computeAllBmi() {
    for (int i = 0; i < count; i++) {
        bmis[i] = computeBmi(weights[i], heights[i]);
    }
}

void SHealth::computeAgeCohortRatios() {
    for (int ageClass = kAgeClassMin; ageClass <= kAgeClassMax; ageClass += kAgeClassStep) {
        int categoryCounts[kBmiCategoryCount] = {0, 0, 0, 0};
        int sum = 0;
        for (int i = 0; i < count; i++) {
            if (!isInAgeCohort(ages[i], ageClass)) {
                continue;
            }
            sum++;
            const int category = classifyBmiCategory(bmis[i]);
            if (category >= 0) {
                categoryCounts[category]++;
            }
        }
        const int cohortIndex = ageClassToCohortIndex(ageClass);
        if (sum == 0) {
            for (int category = 0; category < kBmiCategoryCount; category++) {
                cohortRatios_[cohortIndex][category] = 0.0;
            }
            continue;
        }
        for (int category = 0; category < kBmiCategoryCount; category++) {
            cohortRatios_[cohortIndex][category] =
                static_cast<double>(categoryCounts[category]) * kPercentScale / sum;
        }
    }
}

int SHealth::calculateBmi(const std::string& filename) {
    count = 0;
    for (auto& cohort : cohortRatios_) {
        cohort.fill(0.0);
    }
    if (!loadFromCsv(filename)) {
        return 0;
    }

    imputeMissingWeights();
    computeAllBmi();
    computeAgeCohortRatios();
    return count;
}

double SHealth::getBmiRatio(int ageClass, int type) {
    const int cohortIndex = ageClassToCohortIndex(ageClass);
    const int categoryIndex = typeToCategoryIndex(type);
    if (cohortIndex < 0 || categoryIndex < 0) {
        return 0.0;
    }
    return cohortRatios_[cohortIndex][categoryIndex];
}

std::vector<std::string> SHealth::split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
