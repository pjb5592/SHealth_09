# SHealth BMI — 결함 목록

| 항목 | 내용 |
|------|------|
| 문서 목적 | 요구사항·정적 분석·ctest·`SHealthBMI` 출력 대비 결함 식별·수정·회귀 TC 매핑 |
| 기준 문서 | `README.md`, `docs/requirements_analysis.md`, `docs/code_quality_report.md` |
| 분석 대상 | `src/main/cpp/SHealth.h`, `SHealth.cpp` |
| 검증 | `cmake --build build && ctest` (49/49 Green), `SHealthBMI` ↔ `docs/refactor_baseline_output.txt` 일치 |
| 워크플로우 | **4단계** (결함 분석·수정) |

---

## 요약

| 상태 | 건수 | ID |
|------|------|-----|
| Fixed (5단계·4단계) | 7 | DEF-001 ~ DEF-008 |
| Deferred | 0 | — |

---

## DEF-001 — BMI=25.0 미분류

| 필드 | 내용 |
|------|------|
| **Severity** | High |
| **ItemType** | Functional |
| **Status** | Fixed (5단계 TDD) |

**Steps**

1. `classifyBmiCategory(25.0)` 호출 (또는 BMI=25인 레코드로 `calculateBmi` 후 집계).

**Expected**

- README §3.4: BMI ≥ 25 → **비만**.

**Actual (수정 전)**

- 구현이 `bmis[i] > 25`만 비만 처리 → BMI=25.0은 과체중·비만 모두 해당 없음 → 집계 누락, 4범주 합 &lt; `sum`.

**Root Cause**

- 비만 하한에 **등호 누락** (`>` vs `≥`).

**Fix Summary**

- `classifyBmiCategory`: `bmi < 25` → 과체중, **그 외(≥25)** → 비만 (`else` → `BmiCategoryIndex::Obesity`).

**재발 방지 TC:** TP-P2-07, TP-P3-11, EX-04

---

## DEF-002 — 연령대 무인원(sum=0) 시 0 나누기

| 필드 | 내용 |
|------|------|
| **Severity** | High |
| **ItemType** | Reliability |
| **Status** | Fixed (5단계 TDD) |

**Steps**

1. 해당 연령대 인원 0명인 CSV로 `calculateBmi`.
2. `getBmiRatio(ageClass, type)` 조회.

**Expected**

- 비율 0%, 0 나누기·NaN 없음 (§5.3 R-03).

**Actual (수정 전)**

- `(double)count * 100 / sum`에서 `sum==0` → 미정의 동작.

**Root Cause**

- `computeAgeCohortRatios`에 `sum==0` 분기 없음.

**Fix Summary**

- `if (sum == 0)` 시 해당 코호트 4범주 비율을 `0.0`으로 설정 후 `continue`.

**재발 방지 TC:** TP-P3-10, EX-01

---

## DEF-003 — 연령대 전원 weight=0 시 ageCount=0 나누기

| 필드 | 내용 |
|------|------|
| **Severity** | High |
| **ItemType** | Reliability |
| **Status** | Fixed (5단계 TDD) |

**Steps**

1. 한 연령대 모든 레코드 `weight==0.0`인 픽스처로 `calculateBmi`.

**Expected**

- 보정 스킵 또는 정의된 안전 동작, **0 나누기 없음** (§4.3).

**Actual (수정 전)**

- `weights[i] = sum / ageCount`에서 `ageCount==0` → UB.

**Root Cause**

- 유효 체중 집계 후 `ageCount` 검사 없이 평균 대입.

**Fix Summary**

- `if (ageCount == 0) continue;` 후에만 0 체중 레코드에 평균 대입.

**재발 방지 TC:** TP-P1-04, EX-02

---

## DEF-004 — calculateBmi 실패 시 이전 cohortRatios_ 잔존

| 필드 | 내용 |
|------|------|
| **Severity** | Medium |
| **ItemType** | Functional |
| **Status** | Fixed (5단계 TDD) |

**Steps**

1. 유효 파일로 `calculateBmi` → `getBmiRatio` &gt; 0 확인.
2. 존재하지 않는 파일로 재호출.

**Expected**

- 실패 시 `getBmiRatio` → 0.0 (또는 미집계 상태).

**Actual (수정 전)**

- `count`만 0으로 두고 24개 비율 멤버는 **이전 값 유지**.

**Root Cause**

- 실패 경로에서 집계 버퍼 미초기화.

**Fix Summary**

- `calculateBmi` 시작 시 `count=0`, `cohortRatios_.fill(0.0)`.

**재발 방지 TC:** EX-06 (`CalculateBmi_FailedLoad_ClearsPreviousRatios`)

---

## DEF-005 — height=0 미보정·BMI 무한대

| 필드 | 내용 |
|------|------|
| **Severity** | Medium |
| **ItemType** | Functional |
| **Status** | Fixed (5단계, README Activities 4 F-03) |

**Steps**

1. `height==0.0`, `weight>0` 레코드로 `calculateBmi`.

**Expected**

- 동일 연령대 유효 키 평균으로 보정 후 유한 BMI (§4.4).

**Actual**

- 보정 없음 → `computeBmi`에서 키(m)=0 → **inf/NaN** BMI 가능, 집계 왜곡.

**Root Cause**

- `imputeMissingWeights`만 구현, height 대칭 보정·`computeBmi` 방어 없음.

**Fix Summary**

- `imputeMissingHeights()` 추가 (`shealth::detail::impute::fillHeightZeros`), 파이프라인: 체중 보정 → 키 보정 → BMI.

**재발 방지 TC:** TP-P1-07, TP-P1-08, TP-P0-06

---

## DEF-006 — CSV 빈 줄 시 조기 종료

| 필드 | 내용 |
|------|------|
| **Severity** | Low |
| **ItemType** | Functional |
| **Status** | Fixed (4단계) |

**Steps**

1. 헤더 다음 **빈 줄 1개**, 이후 유효 데이터 2행 CSV로 `calculateBmi`.

**Expected**

- 빈 줄 무시, 이후 2건 로드 (D-02).

**Actual (수정 전)**

- `tokens.empty()` 시 `break` → **이후 모든 행 무시**.

**Root Cause**

- 빈 줄을 EOF로 오인하는 `break`.

**Fix Summary**

- `break` → `continue` (`loadFromCsv`).

**재발 방지 TC:** TP-P3-18 (`CalculateBmi_BlankLineAfterHeader_LoadsFollowingRows`)

---

## DEF-007 — count ≥ kMaxRecords 미검사

| 필드 | 내용 |
|------|------|
| **Severity** | Low |
| **ItemType** | Reliability |
| **Status** | Fixed (4단계) |

**Steps**

1. `kMaxRecords`(10000) 초과 행이 있는 CSV 로드.

**Expected**

- 초과 분 거부 또는 안전 cap, **버퍼 오버플로우 없음**.

**Actual (수정 전)**

- `count++` 무검사 → 고정 배열 OOB 위험.

**Root Cause**

- 로드 루프에 상한 검사 없음.

**Fix Summary**

- `count >= kMaxRecords` 시 추가 적재 중단(`break`). (`shealth.dat` ≈4822건 — 동작·baseline 불변)

**재발 방지 TC:** `CalculateBmi_ExceedsMaxRecords_CapsAtLimit`, `CalculateBmi_ShealthDat_RecordCountWithinCap` (Phase 7)

---

## DEF-008 — CSV parse 예외 미처리

| 필드 | 내용 |
|------|------|
| **Severity** | Medium |
| **ItemType** | Reliability |
| **Status** | Fixed (4단계) |

**Steps**

1. `age` 필드가 비숫자인 CSV로 `calculateBmi`.

**Expected**

- 크래시 없이 실패 처리 (`calculateBmi` → 0).

**Actual (수정 전)**

- `std::stoi`/`std::stod` 예외가 상위로 전파 → **프로세스 종료** 가능.

**Root Cause**

- 파싱 예외 catch·열 개수 검증 없음.

**Fix Summary**

- `tokens.size() < 4` → `return false`.
- `stoi`/`stod`를 `try/catch (const std::exception&)`로 감싸 실패 시 `return false`.

**재발 방지 TC:** TP-P3-17 (`CalculateBmi_BadParse_ReturnsZero`)

---

## 경계값 분석 (요구사항 대비 — 4단계 확정)

| 항목 | README / 요구 | 현행 코드 | 비고 |
|------|---------------|-----------|------|
| BMI 18.5 | 저체중 | `<= 18.5` 저체중 | TC-38·TP-P2-02 Green |
| BMI 23.0 | 과체중 | `< 23` 실패 후 `< 25` | TP-P2-05 Green |
| BMI 25.0 | 비만 | else 비만 | DEF-001 Fixed |
| 연령 [20,30) 등 | 하한 포함·상한 미포함 | `isInAgeCohort` | TP-P3-01~05 Green |
| weight=0 | 연령대 평균 | `imputeMissingWeights` | DEF-003 Fixed |
| sum=0 | 0% | `computeAgeCohortRatios` | DEF-002 Fixed |
| height=0 | 연령대 평균 | `imputeMissingHeights` | DEF-005 Fixed |

---

## test_plan §12 매핑

| DEF-ID | 재발 방지 TC | 우선순위 | 상태 |
|--------|--------------|----------|------|
| DEF-001 | TP-P2-07, TP-P3-11, EX-04 | P2 | Green |
| DEF-002 | TP-P3-10, EX-01 | P3 | Green |
| DEF-003 | TP-P1-04, EX-02 | P1 | Green |
| DEF-004 | EX-06 | P3 | Green |
| DEF-005 | TP-P1-07, TP-P1-08, TP-P0-06 | P1 | Green |
| DEF-006 | TP-P3-18 | P3 | Green |
| DEF-007 | `CalculateBmi_ExceedsMaxRecords_CapsAtLimit` | P3 | Green |
| DEF-008 | TP-P3-17 | P3 | Green |

---

*문서 버전: 1.1 | 워크플로우 5단계(Activities 4) | 다음: `refactoring_plan.md` (6단계)*
