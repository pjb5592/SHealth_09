#pragma once

#include <array>
#include <string>
#include <vector>

#include "BmiDomain.h"
#include "PersonRecord.h"

class SHealthTestPeer;

class SHealth {
    friend class SHealthTestPeer;
public:
    using BmiCategoryType = shealth::domain::BmiCategoryType;

    int calculateBmi(const std::string& filename);
    double getBmiRatio(int ageClass, int type) const;
    double getOverallBmiRatio(int type) const;
    std::vector<int> getNormalBmiUserIds() const;

private:
    static constexpr int kMaxRecords = 10000;
    static constexpr int kAgeCohortCount = shealth::domain::kAgeCohortCount;
    static constexpr int kBmiCategoryCount = shealth::domain::kBmiCategoryCount;

    using BmiCategoryIndex = shealth::domain::BmiCategoryIndex;

    std::vector<PersonRecord> records_;

    std::array<std::array<double, kBmiCategoryCount>, kAgeCohortCount> cohortRatios_{};
    std::array<double, kBmiCategoryCount> overallRatios_{};

    bool loadFromCsv(const std::string& filename);
    void imputeMissingWeights();
    void imputeMissingHeights();
    void computeAllBmi();
    void computeAgeCohortRatios();
    void computeOverallRatios();

    static bool isInAgeCohort(int age, int ageClass);
    static int ageClassToCohortIndex(int ageClass);
    static int typeToCategoryIndex(int type);
    static int classifyBmiCategory(double bmi);
    static double computeBmi(double weightKg, double heightCm);

    std::vector<std::string> split(const std::string& line, char delimiter);
};
