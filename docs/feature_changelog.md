# SHealth BMI — 기능 변경 로그 (Activities 4)

| 항목 | 내용 |
|------|------|
| 단계 | README Activities 4 / 워크플로우 5단계 |
| 검증 | `cmake --build build && ctest` — **47/47 Green** |

---

## F-01 — SRP 책임 분리

`SHealth.cpp`에 `shealth::detail` namespace 도입:

| namespace | 책임 |
|-----------|------|
| `detail::csv` | CSV 파싱, `split` |
| `detail::impute` | 연령대별 weight/height 0 평균 보정 |
| `detail::bmi` | BMI 계산, 4단계 분류, 연령 코호트 헬퍼 |
| `detail::stats` | 연령대·전체 비율 집계 |

`SHealth`는 파사드: `calculateBmi` 오케스트레이션 및 조회 API.

---

## F-02 — 연령대 BMI 분포 비율

기존 `getBmiRatio(int ageClass, int type)` 유지. 내부 집계를 `detail::stats::computeAgeCohortRatios`로 이전. `shealth.dat` baseline 회귀(TP-P3-12) 동일.

---

## F-03 — Height=0 연령대 평균 보정

- `imputeMissingHeights()` — weight=0과 대칭 (`height != 0` 평균, 전원 0 시 스킵)
- 파이프라인 순서: **체중 보정 → 키 보정 → BMI**
- DEF-005 Fixed

---

## F-04 — 정상 BMI 사용자 ID 목록

```cpp
std::vector<int> getNormalBmiUserIds() const;
```

- 정상: `18.5 < BMI < 23` (`classifyBmiCategory` Normal)
- CSV `id` 필드 파싱 (`ids[]` 저장)
- 로드 순서 유지

---

## F-05 — 전체 사용자 BMI 범주 비율

```cpp
double getOverallBmiRatio(int type) const;
```

- `type`: 100/200/300/400 (기존 `BmiCategoryType`과 동일)
- 분모: 전체 `count`
- `count == 0` 또는 잘못된 `type` → `0.0`

---

## 신규 테스트

| ID | 테스트 |
|----|--------|
| TP-P1-07 | `ImputeHeight_SingleZeroIn40s_ReplacesWith170` |
| TP-P1-08 | `ImputeHeight_AllZeroInCohort_LeavesZeroNoCrash` |
| TP-P0-06 | `CalculateBmi_HeightZeroAfterImpute_ReturnsFiniteBmi` |
| TP-F04-01~03 | 정상 목록 혼합·빈·경계 |
| TP-F05-01~02 | 전체 비율 25%×4·빈 데이터 |

픽스처: `height0_single_40s.csv`, `height0_all_zero_40s.csv`, `normal_users_*.csv`, `overall_four_categories.csv`
