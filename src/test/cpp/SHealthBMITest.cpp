#include <cmath>
#include <fstream>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include "SHealth.h"

namespace {

constexpr double kBmiEpsilon = 1e-5;
constexpr double kRatioEpsilon = 1e-3;
constexpr double kRatioSumEpsilon = 0.01;

std::string FixturePath(const std::string& name) {
    return std::string(SHEALTH_FIXTURE_DIR) + "/" + name;
}

std::string ProjectPath(const std::string& name) {
    return std::string(SHEALTH_PROJECT_ROOT) + "/" + name;
}

}  // namespace

class SHealthTestPeer {
public:
    static double ComputeBmi(double weightKg, double heightCm) {
        return SHealth::computeBmi(weightKg, heightCm);
    }

    static int ClassifyBmiCategory(double bmi) { return SHealth::classifyBmiCategory(bmi); }

    static bool IsInAgeCohort(int age, int ageClass) {
        return SHealth::isInAgeCohort(age, ageClass);
    }

    static double WeightAt(const SHealth& health, int index) { return health.weights[index]; }

    static double HeightAt(const SHealth& health, int index) { return health.heights[index]; }

    static int IdAt(const SHealth& health, int index) { return health.ids[index]; }

    static double BmiAt(const SHealth& health, int index) { return health.bmis[index]; }
};

class SHealthFixture : public ::testing::Test {
protected:
    void SetUp() override { health_ = std::make_unique<SHealth>(); }

    int LoadFixture(const std::string& fileName) {
        return health_->calculateBmi(FixturePath(fileName));
    }

    std::unique_ptr<SHealth> health_;
};

// --- P0: BMI 계산 ---

// TP-P0-01: 표준 BMI (70kg, 170cm)
TEST(SHealthBMITest, ComputeBmi_StandardWeightHeight_ReturnsNear2422) {
    // Given: 체중 70kg, 키 170cm
    // When: BMI를 계산하면
    // Then: 약 24.221453
    const double bmi = SHealthTestPeer::ComputeBmi(70.0, 170.0);
    EXPECT_NEAR(bmi, 24.221453, kBmiEpsilon);
}

// TP-P0-02: cm→m 변환 (80kg, 180cm)
TEST(SHealthBMITest, ComputeBmi_CmToMeterConversion_ReturnsNear2469) {
    // Given: 체중 80kg, 키 180cm
    // When: BMI를 계산하면
    // Then: 80 / 1.8²
    const double bmi = SHealthTestPeer::ComputeBmi(80.0, 180.0);
    EXPECT_NEAR(bmi, 80.0 / (1.8 * 1.8), kBmiEpsilon);
}

// TP-P0-03: README 유사 소수 체중·키
TEST(SHealthBMITest, ComputeBmi_ReadmeLikeDecimals_ReturnsExpected) {
    // Given: README 유사 소수 샘플
    // When: BMI를 계산하면
    // Then: 이론값과 일치
    const double bmi = SHealthTestPeer::ComputeBmi(79.5, 158.3);
    const double heightM = 158.3 / 100.0;
    EXPECT_NEAR(bmi, 79.5 / (heightM * heightM), kBmiEpsilon);
}

// TP-P0-04: 저체중 구간 BMI
TEST(SHealthBMITest, ComputeBmi_UnderweightRange_ReturnsNear1730) {
    // Given: 저체중 구간 체중·키
    // When: BMI를 계산하면
    // Then: 약 17.30
    const double bmi = SHealthTestPeer::ComputeBmi(50.0, 170.0);
    EXPECT_NEAR(bmi, 17.301038, kBmiEpsilon);
}

// TP-P0-05: 극단 키 (50kg, 200cm) 유한 양수
TEST(SHealthBMITest, ComputeBmi_TallHeight_ReturnsFinitePositive) {
    // Given: 극단적으로 큰 키
    // When: BMI를 계산하면
    // Then: 유한한 양수
    const double bmi = SHealthTestPeer::ComputeBmi(50.0, 200.0);
    EXPECT_TRUE(std::isfinite(bmi));
    EXPECT_GT(bmi, 0.0);
}

// --- P2: BMI 4단계 분류 ---

namespace BmiCat {
constexpr int kUnderweight = 0;
constexpr int kNormal = 1;
constexpr int kOverweight = 2;
constexpr int kObesity = 3;
}  // namespace BmiCat

// TP-P2-01: BMI 17.0 → 저체중
TEST(SHealthBMITest, ClassifyBmi_17_ReturnsUnderweight) {
    // Given: BMI 17.0
    // When: 분류하면
    // Then: 저체중
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(17.0), BmiCat::kUnderweight);
}

// TP-P2-02: BMI 18.5 → 저체중 (README·TC-38)
TEST(SHealthBMITest, ClassifyBmi_18_5_ReturnsUnderweight) {
    // Given: BMI 18.5 (경계)
    // When: 분류하면
    // Then: 저체중 (README 기준)
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(18.5), BmiCat::kUnderweight);
}

// TP-P2-03: BMI 18.500001 → 정상
TEST(SHealthBMITest, ClassifyBmi_18_500001_ReturnsNormal) {
    // Given: BMI 18.500001
    // When: 분류하면
    // Then: 정상
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(18.500001), BmiCat::kNormal);
}

// TP-P2-04: BMI 22.999 → 정상
TEST(SHealthBMITest, ClassifyBmi_22_999_ReturnsNormal) {
    // Given: BMI 22.999
    // When: 분류하면
    // Then: 정상
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(22.999), BmiCat::kNormal);
}

// TP-P2-05: BMI 23.0 → 과체중
TEST(SHealthBMITest, ClassifyBmi_23_ReturnsOverweight) {
    // Given: BMI 23.0 (경계)
    // When: 분류하면
    // Then: 과체중
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(23.0), BmiCat::kOverweight);
}

// TP-P2-06: BMI 24.999 → 과체중
TEST(SHealthBMITest, ClassifyBmi_24_999_ReturnsOverweight) {
    // Given: BMI 24.999
    // When: 분류하면
    // Then: 과체중
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(24.999), BmiCat::kOverweight);
}

// TP-P2-07: BMI 25.0 → 비만 (EX-04 회귀)
TEST(SHealthBMITest, ClassifyBmi_25_ReturnsObesity) {
    // Given: BMI 25.0 (비만 하한, DEF 회귀)
    // When: 분류하면
    // Then: 비만
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(25.0), BmiCat::kObesity);
}

// TP-P2-08: BMI 30.0 → 비만
TEST(SHealthBMITest, ClassifyBmi_30_ReturnsObesity) {
    // Given: BMI 30.0
    // When: 분류하면
    // Then: 비만
    EXPECT_EQ(SHealthTestPeer::ClassifyBmiCategory(30.0), BmiCat::kObesity);
}

// --- P1: 연령대 평균 보정 (weight=0) ---

// TP-P1-01: 20대 단일 체중 0 → 동일 연령대 평균(60) 대체
TEST_F(SHealthFixture, ImputeWeight_SingleZeroIn20s_ReplacesWith60) {
    // Given: 20대 60kg 1명과 체중 0 1명
    // When: calculateBmi로 보정하면
    // Then: 0 레코드 체중이 60
    ASSERT_EQ(LoadFixture("weight0_single_20s.csv"), 2);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::WeightAt(*health_, 1), 60.0);
}

// TP-P1-02: 30대 다중 유효 체중 평균 → 0 레코드 90
TEST_F(SHealthFixture, ImputeWeight_MultiZeroIn30s_ReplacesWith90) {
    // Given: 30대 80·100·0
    // When: 보정하면
    // Then: 0 레코드가 90
    ASSERT_EQ(LoadFixture("weight0_multi_30s.csv"), 3);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::WeightAt(*health_, 2), 90.0);
}

// TP-P1-03: 연령대 격리 — 타 연령대 평균 미혼입 (TC-37)
TEST_F(SHealthFixture, ImputeWeight_CrossCohort_DoesNotMixAverages) {
    // Given: 20대 0, 30대 70만 유효
    // When: 보정하면
    // Then: 20대 0은 30대 평균에 영향받지 않음
    ASSERT_EQ(LoadFixture("weight0_cohort_isolation.csv"), 2);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::WeightAt(*health_, 0), 0.0);
}

// TP-P1-04: 연령대 전원 weight=0 → 보정 스킵·0 나누기 없음 (EX-02)
TEST_F(SHealthFixture, ImputeWeight_AllZeroInCohort_LeavesZeroNoCrash) {
    // Given: 30대 전원 체중 0
    // When: 보정하면
    // Then: 0 나누기 없이 0 유지, BMI 계산 가능
    ASSERT_EQ(LoadFixture("weight0_all_zero_30s.csv"), 2);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::WeightAt(*health_, 0), 0.0);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::WeightAt(*health_, 1), 0.0);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::BmiAt(*health_, 0), 0.0);
}

// TP-P1-05: 연령대 외(19세) 체중 0 — 20대 평균 미적용
TEST_F(SHealthFixture, ImputeWeight_Age19Zero_Uses20sCohortOnly) {
    // Given: 19세 체중 0, 25세 60kg
    // When: 보정하면
    // Then: 19세는 20대 평균(60) 미적용
    ASSERT_EQ(LoadFixture("age19_weight0.csv"), 2);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::WeightAt(*health_, 0), 0.0);
}

// TP-P1-06: shealth.dat 체중 0 레코드 — 크래시 없음·유한 BMI (TC-19)
TEST_F(SHealthFixture, CalculateBmi_RealDataWeightZero_DoesNotCrash) {
    // Given: shealth.dat 내 체중 0 레코드 포함
    // When: calculateBmi 실행
    // Then: 크래시 없이 유한 BMI
    SHealth health;
    const int count = health.calculateBmi(ProjectPath("shealth.dat"));
    ASSERT_GT(count, 0);
    for (int i = 0; i < count; ++i) {
        const double bmi = SHealthTestPeer::BmiAt(health, i);
        if (SHealthTestPeer::WeightAt(health, i) > 0.0) {
            EXPECT_TRUE(std::isfinite(bmi));
        }
    }
}

// --- P3: 연령대·비율·getBmiRatio ---

// TP-P3-01: age=20 → 20대 [20,30) 포함 (TC-12)
TEST(SHealthBMITest, IsInAgeCohort_Age20In20s_ReturnsTrue) {
    // Given: 나이 20
    // When: 20대 구간 판별
    // Then: 포함
    EXPECT_TRUE(SHealthTestPeer::IsInAgeCohort(20, 20));
}

// TP-P3-02: age=29 → 20대 상한 포함 (TC-13)
TEST(SHealthBMITest, IsInAgeCohort_Age29In20s_ReturnsTrue) {
    // Given: 나이 29
    // When: 20대 구간 판별
    // Then: 포함
    EXPECT_TRUE(SHealthTestPeer::IsInAgeCohort(29, 20));
}

// TP-P3-03: age=30 → 20대 제외·30대 포함 (TC-14)
TEST(SHealthBMITest, IsInAgeCohort_Age30_NotIn20s_In30s) {
    // Given: 나이 30
    // When: 20대·30대 판별
    // Then: 20대 제외, 30대 포함
    EXPECT_FALSE(SHealthTestPeer::IsInAgeCohort(30, 20));
    EXPECT_TRUE(SHealthTestPeer::IsInAgeCohort(30, 30));
}

// TP-P3-04: age=19 → 20대 집계 미포함 (TC-15)
TEST(SHealthBMITest, IsInAgeCohort_Age19_NotIn20s) {
    // Given: 나이 19
    // When: 20대 판별
    // Then: 미포함
    EXPECT_FALSE(SHealthTestPeer::IsInAgeCohort(19, 20));
}

// TP-P3-05: age=79 → 70대 [70,80) 포함 (TC-16)
TEST(SHealthBMITest, IsInAgeCohort_Age79_In70s) {
    // Given: 나이 79
    // When: 70대 판별
    // Then: 포함
    EXPECT_TRUE(SHealthTestPeer::IsInAgeCohort(79, 70));
}

// TP-P3-06: 20대 4범주 각 25%·합 ≈100% (TC-23, R-01, R-02)
TEST_F(SHealthFixture, GetBmiRatio_FourCategoriesEach25Percent) {
    // Given: 20대 4명 각 BMI 범주 1명
    // When: getBmiRatio 조회
    // Then: 각 25%, 합 ≈ 100%
    ASSERT_EQ(LoadFixture("cohort20_four_categories.csv"), 4);
    const double under = health_->getBmiRatio(20, 100);
    const double normal = health_->getBmiRatio(20, 200);
    const double over = health_->getBmiRatio(20, 300);
    const double obese = health_->getBmiRatio(20, 400);
    EXPECT_NEAR(under, 25.0, kRatioEpsilon);
    EXPECT_NEAR(normal, 25.0, kRatioEpsilon);
    EXPECT_NEAR(over, 25.0, kRatioEpsilon);
    EXPECT_NEAR(obese, 25.0, kRatioEpsilon);
    EXPECT_NEAR(under + normal + over + obese, 100.0, kRatioSumEpsilon);
}

// TP-P3-07: 20대 저체중 2/10 → getBmiRatio(20,100)=20% (TC-24)
TEST_F(SHealthFixture, GetBmiRatio_Underweight2Of10_Returns20) {
    // Given: 20대 10명 중 저체중 2명
    // When: type 100 조회
    // Then: 20.0%
    ASSERT_EQ(LoadFixture("cohort20_underweight_2_of_10.csv"), 10);
    EXPECT_NEAR(health_->getBmiRatio(20, 100), 20.0, kRatioEpsilon);
}

// TP-P3-08: 잘못된 type=999 → 0.0 (TC-26)
TEST_F(SHealthFixture, GetBmiRatio_InvalidType_ReturnsZero) {
    // Given: 유효 데이터 로드 후
    // When: 잘못된 type 조회
    // Then: 0.0
    ASSERT_GT(LoadFixture("cohort20_four_categories.csv"), 0);
    EXPECT_DOUBLE_EQ(health_->getBmiRatio(20, 999), 0.0);
}

// TP-P3-09: 잘못된 ageClass=25 → 0.0 (TC-27)
TEST_F(SHealthFixture, GetBmiRatio_InvalidAgeClass_ReturnsZero) {
    // Given: 유효 데이터 로드 후
    // When: 잘못된 ageClass 조회
    // Then: 0.0
    ASSERT_GT(LoadFixture("cohort20_four_categories.csv"), 0);
    EXPECT_DOUBLE_EQ(health_->getBmiRatio(25, 100), 0.0);
}

// TP-P3-10: sum=0 연령대(헤더만 CSV) → 0%·0 나누기 없음 (EX-01, TC-29)
TEST_F(SHealthFixture, GetBmiRatio_EmptyCohort_ReturnsZeroNoDivideByZero) {
    // Given: 헤더만 있는 CSV (20대 무인원)
    // When: 20대 비율 조회
    // Then: 0.0, 예외 없음
    EXPECT_EQ(LoadFixture("header_only.csv"), 0);
    EXPECT_DOUBLE_EQ(health_->getBmiRatio(20, 100), 0.0);
    EXPECT_DOUBLE_EQ(health_->getBmiRatio(20, 200), 0.0);
}

// TP-P3-11: BMI=25.0 1명 → 비만 100% 집계 (EX-04, TC-31)
TEST_F(SHealthFixture, GetBmiRatio_Bmi25CountedAsObesity) {
    // Given: BMI 정확히 25인 20대 1명
    // When: 비만 비율 조회
    // Then: 100% (DEF 회귀)
    ASSERT_EQ(LoadFixture("bmi25_one.csv"), 1);
    EXPECT_NEAR(health_->getBmiRatio(20, 400), 100.0, kRatioEpsilon);
}

// TP-P3-18: 헤더 후 빈 줄 — 이후 데이터 로드 유지 (DEF-006)
TEST_F(SHealthFixture, CalculateBmi_BlankLineAfterHeader_LoadsFollowingRows) {
    // Given: 헤더 다음 빈 줄, 유효 데이터 2행
    // When: calculateBmi 실행
    // Then: 2건 로드, 20대 비율 합산 가능
    ASSERT_EQ(LoadFixture("csv_blank_line_after_header.csv"), 2);
    const double sum = health_->getBmiRatio(20, 100) + health_->getBmiRatio(20, 200) +
                       health_->getBmiRatio(20, 300) + health_->getBmiRatio(20, 400);
    EXPECT_NEAR(sum, 100.0, kRatioSumEpsilon);
}

// TP-P3-17: 잘못된 CSV 숫자 필드 — calculateBmi 0 반환 (DEF-008)
TEST_F(SHealthFixture, CalculateBmi_BadParse_ReturnsZero) {
    // Given: age 필드가 숫자가 아닌 CSV
    // When: calculateBmi 호출
    // Then: 0 반환, 크래시 없음
    EXPECT_EQ(LoadFixture("bad_parse.csv"), 0);
}

// TP-P3-15: 파일 미존재 → calculateBmi 0 반환 (EX-05, TC-28)
TEST_F(SHealthFixture, CalculateBmi_MissingFile_ReturnsZero) {
    // Given: 존재하지 않는 파일
    // When: calculateBmi 호출
    // Then: 0 반환
    EXPECT_EQ(health_->calculateBmi(ProjectPath("no_such_file.dat")), 0);
}

// TP-P3-14: calculateBmi 미호출 → getBmiRatio 0.0
TEST_F(SHealthFixture, GetBmiRatio_WithoutCalculateBmi_ReturnsZero) {
    // Given: calculateBmi 미호출
    // When: getBmiRatio 조회
    // Then: 0.0
    EXPECT_DOUBLE_EQ(health_->getBmiRatio(20, 100), 0.0);
}

// TP-P3-13: 동일 파일 calculateBmi 2회 → idempotent (TC-36)
TEST_F(SHealthFixture, CalculateBmi_TwiceSameFile_IdempotentRatios) {
    // Given: 동일 픽스처
    // When: calculateBmi 두 번 호출
    // Then: 두 번째 결과가 첫 번째와 동일
    const std::string path = FixturePath("cohort20_four_categories.csv");
    ASSERT_EQ(health_->calculateBmi(path), 4);
    const double first = health_->getBmiRatio(20, 100);
    ASSERT_EQ(health_->calculateBmi(path), 4);
    EXPECT_NEAR(health_->getBmiRatio(20, 100), first, kRatioEpsilon);
}

// EX-06: calculateBmi 실패 후 이전 cohortRatios_ 잔존 방지
TEST_F(SHealthFixture, CalculateBmi_FailedLoad_ClearsPreviousRatios) {
    // Given: 먼저 유효 파일 로드
    // When: 실패하는 파일로 재호출
    // Then: 비율 0으로 초기화
    ASSERT_GT(health_->calculateBmi(FixturePath("cohort20_four_categories.csv")), 0);
    ASSERT_GT(health_->getBmiRatio(20, 100), 0.0);
    EXPECT_EQ(health_->calculateBmi(ProjectPath("no_such_file.dat")), 0);
    EXPECT_DOUBLE_EQ(health_->getBmiRatio(20, 100), 0.0);
}

// TP-P3-12: shealth.dat 6연령×4 비율 — baseline 회귀 (TC-40)
TEST_F(SHealthFixture, GetBmiRatio_ShealthDat_MatchesBaseline) {
    // Given: shealth.dat 전체
    // When: calculateBmi 후 6연령×4 비율 조회
    // Then: refactor_baseline_output.txt와 일치
    struct BaselineRow {
        int ageClass;
        double under;
        double normal;
        double over;
        double obese;
    };

    const BaselineRow expected[] = {
        {20, 3.511053, 23.797139, 11.833550, 60.858257},
        {30, 1.863354, 15.527950, 10.062112, 72.546584},
        {40, 0.521512, 10.039113, 9.126467, 80.312907},
        {50, 2.181401, 12.629162, 9.988519, 75.200918},
        {60, 0.862895, 8.533078, 10.642378, 79.961649},
        {70, 0.529101, 12.345679, 10.758377, 76.366843},
    };

    ASSERT_GT(health_->calculateBmi(ProjectPath("shealth.dat")), 0);

    for (const auto& row : expected) {
        EXPECT_NEAR(health_->getBmiRatio(row.ageClass, 100), row.under, kRatioEpsilon)
            << "ageClass=" << row.ageClass << " underweight";
        EXPECT_NEAR(health_->getBmiRatio(row.ageClass, 200), row.normal, kRatioEpsilon)
            << "ageClass=" << row.ageClass << " normal";
        EXPECT_NEAR(health_->getBmiRatio(row.ageClass, 300), row.over, kRatioEpsilon)
            << "ageClass=" << row.ageClass << " overweight";
        EXPECT_NEAR(health_->getBmiRatio(row.ageClass, 400), row.obese, kRatioEpsilon)
            << "ageClass=" << row.ageClass << " obesity";
    }
}

// --- P1: height=0 연령대 평균 보정 ---

// TP-P1-07: 40대 단일 height 0 → 동일 연령대 평균(170) 대체
TEST_F(SHealthFixture, ImputeHeight_SingleZeroIn40s_ReplacesWith170) {
    // Given: 40대 170cm 1명과 키 0 1명
    // When: calculateBmi로 보정하면
    // Then: 0 레코드 키가 170
    ASSERT_EQ(LoadFixture("height0_single_40s.csv"), 2);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::HeightAt(*health_, 1), 170.0);
}

// TP-P1-08: 연령대 전원 height=0 → 보정 스킵·0 나누기 없음
TEST_F(SHealthFixture, ImputeHeight_AllZeroInCohort_LeavesZeroNoCrash) {
    // Given: 40대 전원 키 0
    // When: 보정하면
    // Then: 0 나누기 없이 0 유지
    ASSERT_EQ(LoadFixture("height0_all_zero_40s.csv"), 2);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::HeightAt(*health_, 0), 0.0);
    EXPECT_DOUBLE_EQ(SHealthTestPeer::HeightAt(*health_, 1), 0.0);
}

// TP-P0-06: height=0 보정 후 유한 BMI
TEST_F(SHealthFixture, CalculateBmi_HeightZeroAfterImpute_ReturnsFiniteBmi) {
    // Given: 40대 height 0 1건
    // When: calculateBmi 실행
    // Then: 보정 후 유한 BMI
    ASSERT_EQ(LoadFixture("height0_single_40s.csv"), 2);
    const double bmi = SHealthTestPeer::BmiAt(*health_, 1);
    EXPECT_TRUE(std::isfinite(bmi));
    EXPECT_GT(bmi, 0.0);
}

// --- F-04: 정상 BMI 사용자 목록 ---

// TP-F04-01: 혼합 ID → 정상만 포함
TEST_F(SHealthFixture, GetNormalBmiUserIds_Mixed_ReturnsNormalOnly) {
    // Given: ID 1·3 정상, 2 저체중
    // When: getNormalBmiUserIds 호출
    // Then: {1, 3}
    ASSERT_EQ(LoadFixture("normal_users_mixed.csv"), 3);
    const std::vector<int> ids = health_->getNormalBmiUserIds();
    ASSERT_EQ(ids.size(), 2u);
    EXPECT_EQ(ids[0], 1);
    EXPECT_EQ(ids[1], 3);
}

// TP-F04-02: 전원 비만 → 빈 목록
TEST_F(SHealthFixture, GetNormalBmiUserIds_AllObese_ReturnsEmpty) {
    // Given: 전원 BMI≥25
    // When: getNormalBmiUserIds 호출
    // Then: 빈 목록
    ASSERT_EQ(LoadFixture("normal_users_all_obese.csv"), 3);
    EXPECT_TRUE(health_->getNormalBmiUserIds().empty());
}

// TP-F04-03: 경계 18.5·23.0 제외
TEST_F(SHealthFixture, GetNormalBmiUserIds_Boundaries_ExcludesEdgeBmis) {
    // Given: 18.5·23.0 경계 및 정상 구간 ID
    // When: getNormalBmiUserIds 호출
    // Then: 정상 구간 ID만 (10, 12)
    ASSERT_EQ(LoadFixture("normal_users_boundaries.csv"), 4);
    const std::vector<int> ids = health_->getNormalBmiUserIds();
    ASSERT_EQ(ids.size(), 2u);
    EXPECT_EQ(ids[0], 10);
    EXPECT_EQ(ids[1], 12);
}

// TP-F04: calculateBmi 미호출 → 빈 목록
TEST_F(SHealthFixture, GetNormalBmiUserIds_WithoutCalculateBmi_ReturnsEmpty) {
    // Given: calculateBmi 미호출
    // When: getNormalBmiUserIds 조회
    // Then: 빈 목록
    EXPECT_TRUE(health_->getNormalBmiUserIds().empty());
}

// --- F-05: 전체 사용자 BMI 범주 비율 ---

// TP-F05-01: 4범주 각 25%
TEST_F(SHealthFixture, GetOverallBmiRatio_FourCategoriesEach25Percent) {
    // Given: 4명 각 BMI 범주 1명
    // When: getOverallBmiRatio 조회
    // Then: 각 25%, 합 ≈ 100%
    ASSERT_EQ(LoadFixture("overall_four_categories.csv"), 4);
    const double under = health_->getOverallBmiRatio(100);
    const double normal = health_->getOverallBmiRatio(200);
    const double over = health_->getOverallBmiRatio(300);
    const double obese = health_->getOverallBmiRatio(400);
    EXPECT_NEAR(under, 25.0, kRatioEpsilon);
    EXPECT_NEAR(normal, 25.0, kRatioEpsilon);
    EXPECT_NEAR(over, 25.0, kRatioEpsilon);
    EXPECT_NEAR(obese, 25.0, kRatioEpsilon);
    EXPECT_NEAR(under + normal + over + obese, 100.0, kRatioSumEpsilon);
}

// TP-F05-02: count=0 → 0%
TEST_F(SHealthFixture, GetOverallBmiRatio_EmptyData_ReturnsZero) {
    // Given: 헤더만 CSV
    // When: 전체 비율 조회
    // Then: 0.0
    EXPECT_EQ(LoadFixture("header_only.csv"), 0);
    EXPECT_DOUBLE_EQ(health_->getOverallBmiRatio(100), 0.0);
    EXPECT_DOUBLE_EQ(health_->getOverallBmiRatio(200), 0.0);
}

// TP-F05: 잘못된 type → 0.0
TEST_F(SHealthFixture, GetOverallBmiRatio_InvalidType_ReturnsZero) {
    ASSERT_GT(LoadFixture("overall_four_categories.csv"), 0);
    EXPECT_DOUBLE_EQ(health_->getOverallBmiRatio(999), 0.0);
}
