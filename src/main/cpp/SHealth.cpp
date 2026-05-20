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

bool loadFromCsv(const std::string& filename, std::vector<PersonRecord>& records, int maxRecords) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    records.clear();
    records.reserve(static_cast<std::size_t>(maxRecords));

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        const std::vector<std::string> tokens = split(line, ',');
        if (tokens.empty()) {
            continue;
        }
        if (static_cast<int>(records.size()) >= maxRecords) {
            break;
        }
        if (tokens.size() < 4) {
            return false;
        }
        try {
            PersonRecord record;
            record.id = std::stoi(tokens[0]);
            record.age = std::stoi(tokens[1]);
            record.weight = std::stod(tokens[2]);
            record.height = std::stod(tokens[3]);
            records.push_back(record);
        } catch (const std::exception&) {
            return false;
        }
    }
    return true;
}

}  // namespace csv

namespace impute {

void fillCohortAverage(std::vector<PersonRecord>& records,
                       double PersonRecord::*field,
                       bool (*isMissing)(double)) {
    const std::size_t count = records.size();
    for (int ageClass = bmi::kAgeClassMin; ageClass <= bmi::kAgeClassMax;
         ageClass += bmi::kAgeClassStep) {
        double sum = 0;
        int validCount = 0;
        for (std::size_t i = 0; i < count; i++) {
            if (!bmi::isInAgeCohort(records[i].age, ageClass)) {
                continue;
            }
            const double value = records[i].*field;
            if (isMissing(value)) {
                continue;
            }
            sum += value;
            validCount++;
        }
        if (validCount == 0) {
            continue;
        }
        const double average = sum / validCount;
        for (std::size_t i = 0; i < count; i++) {
            if (!bmi::isInAgeCohort(records[i].age, ageClass)) {
                continue;
            }
            if (isMissing(records[i].*field)) {
                records[i].*field = average;
            }
        }
    }
}

bool isZero(double value) { return value == 0.0; }

void fillWeightZeros(std::vector<PersonRecord>& records) {
    fillCohortAverage(records, &PersonRecord::weight, isZero);
}

void fillHeightZeros(std::vector<PersonRecord>& records) {
    fillCohortAverage(records, &PersonRecord::height, isZero);
}

}  // namespace impute

namespace stats {

constexpr double kPercentScale = 100.0;

void computeAgeCohortRatios(
    const std::vector<PersonRecord>& records,
    std::array<std::array<double, bmi::kBmiCategoryCount>, bmi::kAgeCohortCount>& cohortRatios) {
    const std::size_t count = records.size();
    for (int ageClass = bmi::kAgeClassMin; ageClass <= bmi::kAgeClassMax;
         ageClass += bmi::kAgeClassStep) {
        int categoryCounts[bmi::kBmiCategoryCount] = {0, 0, 0, 0};
        int sum = 0;
        for (std::size_t i = 0; i < count; i++) {
            if (!bmi::isInAgeCohort(records[i].age, ageClass)) {
                continue;
            }
            sum++;
            const int category = bmi::classifyBmiCategory(records[i].bmi);
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

void computeOverallRatios(const std::vector<PersonRecord>& records,
                          std::array<double, bmi::kBmiCategoryCount>& overallRatios) {
    int categoryCounts[bmi::kBmiCategoryCount] = {0, 0, 0, 0};
    const std::size_t count = records.size();
    if (count == 0) {
        overallRatios.fill(0.0);
        return;
    }
    for (std::size_t i = 0; i < count; i++) {
        const int category = bmi::classifyBmiCategory(records[i].bmi);
        if (category >= 0) {
            categoryCounts[category]++;
        }
    }
    for (int category = 0; category < bmi::kBmiCategoryCount; category++) {
        overallRatios[category] =
            static_cast<double>(categoryCounts[category]) * kPercentScale /
            static_cast<double>(count);
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
    return shealth::detail::csv::loadFromCsv(filename, records_, kMaxRecords);
}

void SHealth::imputeMissingWeights() {
    shealth::detail::impute::fillWeightZeros(records_);
}

void SHealth::imputeMissingHeights() {
    shealth::detail::impute::fillHeightZeros(records_);
}

void SHealth::computeAllBmi() {
    for (PersonRecord& record : records_) {
        record.bmi = computeBmi(record.weight, record.height);
    }
}

void SHealth::computeAgeCohortRatios() {
    shealth::detail::stats::computeAgeCohortRatios(records_, cohortRatios_);
}

void SHealth::computeOverallRatios() {
    shealth::detail::stats::computeOverallRatios(records_, overallRatios_);
}

int SHealth::calculateBmi(const std::string& filename) {
    records_.clear();
    for (auto& cohort : cohortRatios_) {
        cohort.fill(0.0);
    }
    overallRatios_.fill(0.0);
    if (!loadFromCsv(filename)) {
        records_.clear();
        return 0;
    }

    imputeMissingWeights();
    imputeMissingHeights();
    computeAllBmi();
    computeAgeCohortRatios();
    computeOverallRatios();
    return static_cast<int>(records_.size());
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
    if (categoryIndex < 0 || records_.empty()) {
        return 0.0;
    }
    return overallRatios_[categoryIndex];
}

std::vector<int> SHealth::getNormalBmiUserIds() const {
    std::vector<int> normalIds;
    if (records_.empty()) {
        return normalIds;
    }
    const int normalIndex = static_cast<int>(BmiCategoryIndex::Normal);
    for (const PersonRecord& record : records_) {
        if (classifyBmiCategory(record.bmi) == normalIndex) {
            normalIds.push_back(record.id);
        }
    }
    return normalIds;
}

std::vector<std::string> SHealth::split(const std::string& line, char delimiter) {
    return shealth::detail::csv::split(line, delimiter);
}
