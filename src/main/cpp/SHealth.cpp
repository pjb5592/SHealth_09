#include "SHealth.h"

#include "CsvLoader.h"
#include "Imputation.h"
#include "Statistics.h"

bool SHealth::isInAgeCohort(int age, int ageClass) {
    return shealth::domain::isInAgeCohort(age, ageClass);
}

int SHealth::ageClassToCohortIndex(int ageClass) {
    return shealth::domain::ageClassToCohortIndex(ageClass);
}

int SHealth::typeToCategoryIndex(int type) {
    return shealth::domain::typeToCategoryIndex(type);
}

int SHealth::classifyBmiCategory(double bmi) {
    return shealth::domain::classifyBmiCategory(bmi);
}

double SHealth::computeBmi(double weightKg, double heightCm) {
    return shealth::domain::computeBmi(weightKg, heightCm);
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

double SHealth::getBmiRatio(int ageClass, BmiCategoryType type) const {
    const int cohortIndex = ageClassToCohortIndex(ageClass);
    const int categoryIndex = typeToCategoryIndex(static_cast<int>(type));
    if (cohortIndex < 0 || categoryIndex < 0) {
        return 0.0;
    }
    return cohortRatios_[cohortIndex][categoryIndex];
}

double SHealth::getBmiRatio(int ageClass, int type) const {
    return getBmiRatio(ageClass, static_cast<BmiCategoryType>(type));
}

double SHealth::getOverallBmiRatio(BmiCategoryType type) const {
    const int categoryIndex = typeToCategoryIndex(static_cast<int>(type));
    if (categoryIndex < 0 || records_.empty()) {
        return 0.0;
    }
    return overallRatios_[categoryIndex];
}

double SHealth::getOverallBmiRatio(int type) const {
    return getOverallBmiRatio(static_cast<BmiCategoryType>(type));
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
