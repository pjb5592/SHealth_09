# SHealth BMI — QA 활동 종합 보고서

| 항목 | 내용 |
|------|------|
| 문서 목적 | 리팩토링 우선(수정) 워크플로우 **0~11단계** 산출물·검증 결과를 통합하고, README Activities **5. 회고** 항목을 반영한 QA 종합 보고 |
| 작성 역할 | QA 리드 엔지니어 |
| 기준일 | 2026-05-20 |
| 검증 시점 | 본 문서 작성 직전 `ctest` 재실행 완료 |
| 연계 문서 | `docs/requirements_analysis.md`, `code_quality_report.md`, `test_plan.md`, `defect_list.md`, `defect_report.md`, `refactoring_plan.md`, `feature_changelog.md`, `architecture.md` |

---

## 0. Executive Summary

| 지표 | 목표 | 실측 (2026-05-20) | 판정 |
|------|------|-------------------|------|
| **CTest 통과율** | 100% (8단계 이후 유지) | **56 / 56 = 100%** | ✅ |
| **결함 인벤토리** | High/Medium 우선 수정 | DEF-001~008 **전건 Fixed** | ✅ |
| **DEF 회귀 TC** | Fixed 건 100% 매핑 | **8 / 8 = 100%** | ✅ |
| **Golden Master** | `shealth.dat` 6연령×4분류 회귀 | 2 TC Green | ✅ |
| **요구 TC (TC-01~40)** | test_plan·요구 §8 매핑 | **40 / 40 대응** (일부 통합·중복 TC) | ✅ |
| **라인 커버리지** | `shealth_lib` .cpp ≥ 90% | **99.0%** (193/195) | ✅ |
| **분기 커버리지** | `shealth_lib` .cpp ≥ 85% | **97.5%** (118/121, throw 제외) | ✅ |
| **실습 목표 (README)** | Activities 1~4 | 완료; Activity 5 회고는 본 문서로 충족 | ✅ |

**한 줄 요약:** 레거시 God Method를 **파사드 + 도메인/I/O/보정/통계 모듈**로 분리하고, **리팩토링·기능 개선 후 TDD**로 8건 결함을 고정한 뒤 Golden Master·커버리지 보강까지 **56개 자동 테스트**로 회귀 기준을 확보했다.

---

## 1. 워크플로우·산출물 맵 (0~11단계)

| 단계 | 내용 | 산출물 | 검증 |
|------|------|--------|------|
| 0 | Cursor 규칙 | `.cursorrules` | 리팩토링 우선·단계별 완료 조건 |
| 1 | 요구사항 분석 | `docs/requirements_analysis.md` | 코드 변경 없음 |
| 2 | 코드 품질 정적 분석 | `docs/code_quality_report.md` | SOLID·스멜·우선순위 1~5 |
| 3 | 1차 리팩토링 (동작 보존) | `SHealth.*`, `refactor_baseline_output.txt` | 빌드 + baseline 6행 일치 |
| 4 | 결함 분석·수정 | `docs/defect_list.md` | 정적 + 수동; 일부 TC 선행 |
| 5 | 기능 개선 (F-01~F-05) | `docs/feature_changelog.md` | SRP·height 보정·신규 API |
| 6 | 2차 리팩토링 로드맵 | `docs/refactoring_plan.md` | Phase 0~7 체크리스트 |
| 7 | 테스트 계획 | `docs/test_plan.md` | P0~P3·§12 DEF 매핑 |
| 8 | TDD 구현 | `SHealthBMITest.cpp` | **ctest Green** |
| 9 | Golden Master | `test/golden/shealth_bmi.approved.txt` | `GoldenMaster_*` 2 TC |
| 10 | 결함 관리 프로세스 | `docs/defect_report.md` | Severity×ItemType·메트릭 |
| 11 | 아키텍처 | `docs/architecture.md` | 클래스·레이어 다이어그램 |
| **12** | **QA 종합 (본 문서)** | `docs/qa_final_report.md` | ctest 재검증 반영 |

**수정 워크플로우 핵심:** README 원순서(리팩토링 → UnitTest → 기능)와 달리 **2(품질) → 3(1차 리팩) → 4(결함) → 5(기능) → 7~8(테스트 계획·TDD) → 9(Golden)** 순으로 진행했다. 테스트는 구조가 잡힌 뒤 **경계값·결함을 코드로 고정**하는 용도로 후반에 집중했다.

---

## 2. 리팩토링 Before & After (2·3·7·8단계)

### 2.1 2단계 — 정적 분석 시점 (Before)

`docs/code_quality_report.md` (1차 리팩토링 **전** 레거시 기준):

| 지표 | 평가 | 핵심 근거 |
|------|------|-----------|
| SRP | 심각 위반 | `calculateBmi` ~100줄 God Method (로드·보정·BMI·집계·24필드 갱신) |
| DRY | 심각 | 연령대 루프 3회 + `a==20/30/…` 6분기 + `getBmiRatio` **24개** `else if` |
| Magic Number | 다수 | `18.5`, `23`, `25`, `100/200/300/400`, `10000` 산재 |
| 경계값 | 결함 후보 3건+ | BMI=25 미분류, `sum==0`·`ageCount==0` 0 나누기, `height==0` |
| 테스트 용이성 | 낮음 | 순수 함수 분리 불가, private 로직 미검증 |

### 2.2 3단계 — 1차 리팩토링 (동작 보존, After v1)

| Before | After (3단계) |
|--------|----------------|
| God Method 단일 블록 | `calculateBmi` **오케스트레이션** + `loadFromCsv` / `imputeMissingWeights` / `computeAllBmi` / `computeAgeCohortRatios` |
| 24개 `underweight20` … 평면 멤버 | `cohortRatios_[6][4]` (`std::array`) |
| `getBmiRatio` 24 `else if` | 코호트·카테고리 **인덱스 조회** |
| 리터럴 경계값 | `constexpr` + `BmiCategoryType` enum |
| `SHealthBMI.cpp` 24회 중복 출력 | 연령대 `for` + enum type |

**검증:** `SHealthBMI` 출력 6행이 `docs/refactor_baseline_output.txt`와 **라인 단위 일치** (리팩토링 전·후 diff 없음). 이 시점 `ctest`는 `FailedTest` 플레이스홀더로 **Red 예상** — 8단계 전까지 Green 필수 아님.

### 2.3 7·8단계 — 테스트 계획·TDD 이후 (After v2)

7단계 `test_plan.md`는 3단계 구조(`SHealthTestPeer`, 픽스처, P0~P3)를 전제로 설계했다. 8단계 TDD에서:

- **Red → Green**으로 DEF-001~004(도메인·집계) 수정
- `friend class SHealthTestPeer`로 `computeBmi`·`classifyBmiCategory` **직접 단위 검증** (방식 A)
- 픽스처 CSV 8종+ → 이후 Golden·F-04/05·cap TC로 확장

### 2.4 6단계 2차 리팩토링 — 최종 구조 (After v3)

`docs/refactoring_plan.md` Phase 0~7 완료 후:

```text
SHealth (파사드, SHealth.cpp 93줄)
  ├── shealth::detail::csv      — CsvLoader, kMaxCsvRecords, istream DIP
  ├── shealth::detail::impute   — weight/height 0 연령대 평균
  ├── shealth::detail::stats    — 연령대·전체 비율
  ├── shealth::domain           — BMI·분류·연령 테이블 (BmiDomain.h)
  └── PersonRecord + vector     — 고정 배열 5개 제거
```

| 항목 | 2단계(레거시) | 최종(12단계) |
|------|---------------|--------------|
| `SHealth.cpp` 규모 | ~148줄 단일 TU | **93줄** 파사드 (+ 모듈 TU 분리) |
| 저장소 | `double[10000]` × 5 | `vector<PersonRecord>` |
| `getBmiRatio` | 24분기 | 테이블 lookup + 2D 배열 |
| Public API | 2개 | **5개** (`getOverallBmiRatio`, `getNormalBmiUserIds` 추가) |
| 자동 테스트 | 0 (플레이스홀더) | **56** |
| 회귀 기준 | 수동 stdout | baseline + **Golden Master** 파일 |

**8단계 이후에도 리팩토링을 계속한 이유:** 테스트가 Green인 상태에서 Phase 1~7을 적용해 **각 Phase마다 ctest로 회귀 차단** — “테스트 없는 구조 변경” 리스크를 줄였다.

---

## 3. 테스트 완료율·커버리지 (4~5·7~9단계, 목표 대비)

### 3.1 CTest 재검증 (12단계, 본 문서 작성 시)

```text
cd D:\DEV\SHealth_09\build
cmake --build .
ctest --output-on-failure

결과: 100% tests passed, 0 tests failed out of 56
Total Test time (real) ≈ 25 sec
```

| 구간 | 테스트 수 | 통과 | 비고 |
|------|-----------|------|------|
| 5단계 TDD 완료 시점 | 35 | 35/35 | Report/05 |
| 6단계 결함 TC 추가 후 | 37 | 37/37 | Report/06 |
| 7단계 F-03~F-05 TC | 47 | 47/47 | feature_changelog |
| 9단계 Golden + Phase 7 cap | 50 | 50/50 | |
| 커버리지 보강 TC | **56** | **56/56** | CsvLoader·리포트·열 부족 CSV |

### 3.2 test_plan 우선순위별 완료율

| 우선순위 | 영역 | 계획 TC 수(목표) | 구현 TEST 수 | 대표 테스트 | 판정 |
|----------|------|------------------|--------------|-------------|------|
| **P0** | BMI 계산 | 5~8 | **6** | `ComputeBmi_*` ×5, `CalculateBmi_HeightZeroAfterImpute_*` | ✅ 목표 상한 |
| **P1** | weight/height 보정 | 8~12 | **9** | `ImputeWeight_*` ×6, `ImputeHeight_*` ×2, 실데이터 스모크 | ✅ |
| **P2** | 4단계 분류 | 10~14 | **8** | `ClassifyBmi_*` (18.5·23·25 경계 포함) | ✅ 핵심 경계 전부 |
| **P3** | 통계·API·파일 | 12~16 | **21+** | `GetBmiRatio_*`, baseline, Golden, cap, F-04/05 | ✅ 목표 초과 |
| **Golden (9단계)** | 전체 리포트 회귀 | 1~2 | **2** | `GoldenMaster_ShealthDat_*` | ✅ |
| **합계** | — | ~35~50 (가이드) | **56** | `SHealthBMITest.cpp` | ✅ |

**요구사항 §8 (TC-01~TC-40):** `test_plan.md` §11 매핑 기준 **40개 시나리오 전부** 대응 TC 존재 (일부는 하나의 TEST가 복수 ID 커버, 예: baseline + Golden).

### 3.3 코드 커버리지 (목표 대비)

**측정 명령 (2026-05-20 재실행)**

```bash
cmake -B build -DSHEALTH_COVERAGE=ON
cmake --build build
cmake --build build --target coverage
```

| 산출 | 경로 |
|------|------|
| 요약 | `build/coverage_summary.txt` |
| 도구 | `cmake/SummarizeGcov.py` (GCC 15 + lcov 비호환 시 gcov `.gcov` 파싱) |

| 파일 | 라인 | 분기 | 판정 |
|------|------|------|------|
| `SHealth.cpp` | **97.1%** (66/68) | 100% (25/25) | ✅ |
| `CsvLoader.cpp` | **100%** (45/45) | 97.8% (45/46) | ✅ |
| `Imputation.cpp` | 100% (29/29) | 100% (16/16) | ✅ |
| `Statistics.cpp` | 100% (37/37) | 91.3% (21/23) | ✅ |
| `SHealthBmiReport.cpp` | 100% (16/16) | 100% (11/11) | ✅ |
| **`shealth_lib` .cpp 합산** | **99.0%** (193/195) | **97.5%** (118/121) | **라인·분기 목표 달성** |

**해석**

1. **라인·분기 목표 달성** — `TP-P3-19`(열 부족 CSV), `CsvLoaderTest` 3건(istream fail·maxRecords=0·split), `Report_Format_*` 2건 추가 후 재측정.
2. **분기 집계** — MinGW gcov의 STL `(throw)` 프로브는 제품 분기가 아니므로 `SummarizeGcov.py`에서 제외; 포함 시 74%대로 과소 평가됨.
3. `BmiDomain.h` 인라인 로직은 `SHealth.cpp` TU에 편입; 단독 `.gcov` 0%는 참고치.
4. lcov HTML은 GCC 15 JSON 비호환 — `coverage_summary.txt`가 공식 요약.

### 3.4 9단계 Golden Master

| 항목 | 내용 |
|------|------|
| Approved 파일 | `test/golden/shealth_bmi.approved.txt` |
| 참고 baseline | `docs/refactor_baseline_output.txt` (3단계와 동일 6행) |
| TC | `GoldenMaster_ShealthDat_ReportMatchesApprovedFile`, `GoldenMaster_ShealthDat_GetBmiRatiosMatchApprovedFile` |
| 갱신 | `cmake --build . --target update-golden-shealth-bmi` (의도된 동작 변경 시만) |

---

## 4. 결함 패턴 — 6단계 정적 vs 5단계 TC

### 4.1 인벤토리 요약

| 상태 | 건수 | ID |
|------|------|-----|
| Fixed | **8** | DEF-001 ~ DEF-008 |
| Deferred | **0** | (DEF-005 height=0 → 5단계 F-03에서 Fixed) |

### 4.2 발견 경로 (`docs/defect_report.md` §4.4 기준, 최종 동기화)

| 발견 경로 | 건수 | 비율 | DEF-ID |
|-----------|------|------|--------|
| **5단계 TDD / TC** | 5 | **62.5%** | DEF-001~005 |
| **6단계 정적 분석** | 3 | **37.5%** | DEF-006~008 |

| Severity | TDD | 정적 |
|----------|-----|------|
| High | 3 | 0 |
| Medium | 2 | 1 |
| Low | 0 | 2 |

| ItemType | TDD | 정적 |
|----------|-----|------|
| Functional | 3 | 1 |
| Reliability | 2 | 2 |

### 4.3 패턴 해석

1. **도메인·경계값 결함**(BMI=25, `sum==0`, 전원 weight=0, 상태 잔존, height=0)은 **TDD Red**에서 먼저 드러났다. `test_plan` §5 경계 매트릭스·P2 `TP-P2-07`이 실질적 안전망이었다.
2. **I/O·방어 코드**(빈 줄 `break`, `kMaxRecords` 상한, `stoi` 예외)는 **요구 D-02·코드 대조 정적 분석**에서 발견되고, **수정 후** TP-P3-17/18·cap TC로 회귀가 고정되었다.
3. **리팩토링 우선 워크플로우 교훈:** 구조 개선(3단계)만으로는 BMI=25 같은 **요구 해석 버그**가 남을 수 있음 → 8단계 TDD가 “동작 보존”의 실제 의미를 검증한다.

### 4.4 대표 결함 ↔ TC (§12 요약)

| DEF | 요약 | 재발 방지 TC |
|-----|------|--------------|
| DEF-001 | BMI=25 미분류 | `ClassifyBmi_25_*`, `GetBmiRatio_Bmi25CountedAsObesity` |
| DEF-002 | sum=0 0 나누기 | `GetBmiRatio_EmptyCohort_*` |
| DEF-003 | 전원 weight=0 | `ImputeWeight_AllZeroInCohort_*` |
| DEF-004 | 실패 시 비율 잔존 | `CalculateBmi_FailedLoad_ClearsPreviousRatios` |
| DEF-005 | height=0 | `ImputeHeight_*`, `CalculateBmi_HeightZeroAfterImpute_*` |
| DEF-006 | 빈 줄 break | `CalculateBmi_BlankLineAfterHeader_*` |
| DEF-007 | 레코드 상한 | `CalculateBmi_ExceedsMaxRecords_*` |
| DEF-008 | 파싱 예외 | `CalculateBmi_BadParse_*` |

---

## 5. README Activities 5 — 회고

### 5.1 실습 목표와 달성도

| README Activity | 목표 | 달성 | 근거 |
|-----------------|------|------|------|
| 1. 코드 분석·스멜 | BMI 이해, 스멜 식별 | ✅ | `requirements_analysis.md`, `code_quality_report.md` |
| 2. 1차 리팩토링 | 네이밍·상수·함수 추출·DRY | ✅ | 3단계, baseline 동일 |
| 3. UnitTest | BMI·보정·분류·예외 TC | ✅ (후반) | 7~8단계, **50 TC** |
| 4. 기능 개선 | SRP·height·목록·전체 비율 | ✅ | F-01~F-05, `feature_changelog.md` |
| 5. 회고 | Before/After·AI·TC 팁 | ✅ | **본 문서 §5~§7** |

**수정 워크플로우 달성:** “리팩토링·기능을 먼저, 테스트로 고정은 나중” 전략을 따랐고, 최종적으로 **자동 회귀(56 TC + Golden + coverage)** 까지 확보했다.

### 5.2 코드 품질 Before & After (요약)

| 관점 | Before (레거시) | After (최종) |
|------|-----------------|--------------|
| 가독성 | 100줄 단일 메서드, 매직 넘버 | 모듈·상수·enum·짧은 파사드 |
| 변경 용이성 | 연령/범주 추가 시 24곳 수정 | 테이블·벡터·루프 |
| 안정성 | 경계 버그·0 나누기·파싱 크래시 | DEF 8건 수정 + TC |
| 검증 | 수동 stdout | ctest + Golden 파일 |
| AI 협업 적합성 | 맥락 과다 단일 파일 | 단계별 `docs/` + 작은 TU |

### 5.3 AI 활용 효과·한계 — 1차 리팩토링 후 TDD 선행 효과

**효과가 컸던 순간**

| 활용 | 효과 |
|------|------|
| 요구·품질 문서 초안 (`requirements_analysis`, `code_quality_report`) | 도메인 경계(18.5/23/25)·스멜 우선순위를 **사람 리뷰 전에 구조화** |
| 1차 리팩토링 (3단계) | God Method 분해·`cohortRatios_` 제안을 **빠르게 적용**, baseline으로 동작 보존 확인 |
| `test_plan` + TDD (7~8단계) | Given-When-Then·TP-ID 체계로 **Red 테스트 대량 생성** → DEF-001~004를 짧은 사이클로 수정 |
| 2차 Phase 체크리스트 (`refactoring_plan`) | Phase마다 “Green 유지” 조건을 AI가 상기 → **49→50 TC** 유지하며 모듈 분리 |
| Golden Master (9단계) | 리포트 포맷 추출·approved 파일·CMake 타깃 **보일러플레이트 자동화** |

**1차 리팩토링 *후* TDD를 둔 효과 (수정 워크플로우 핵심)**

- private/static 로직이 **함수 단위로 이미 분리**되어 `SHealthTestPeer`·픽스처 테스트 작성이 수월했다.
- “출력이 같다”는 수동 baseline만으로는 잡히지 않던 **BMI=25·sum=0**이 Red로 드러났다.
- 기능 개선(F-03 height) 시 **기존 P1/P2 TC 확장**만으로 회귀 범위를 한정할 수 있었다.

**한계·주의**

| 한계 | 대응 |
|------|------|
| 동작 보존 리팩토링 중 **버그를 보존**할 위험 | 4단계 결함 목록 + 8단계 경계 TC 필수 |
| AI가 README vs `bmi.png` 경계를 혼동 | `requirements_analysis` §3.3에서 **README 우선 확정** |
| `Statistics.cpp` 분기 91.3% | 잔여 2 분기(throw 제외 후) — 선택적 보강 |
| 문서·실측 수치 불일치 (35→50 TC) | **12단계에서 ctest 재실행**으로 본 문서 기준선 고정 |

### 5.4 TC 추가가 개선에 미친 영향

- **결함 수정의 트리거:** TDD Red가 DEF-001~003의 수정 우선순위를 객관화했다.
- **리팩토링 안전망:** Phase 1~7 구조 변경 시 **56 TC가 약 25초 내** 회귀 피드백.
- **API 계약 고정:** `getBmiRatio` 잘못된 인자 → 0.0, `calculateBmi` 실패 → 0 등 **미정의 동작을 TC로 문서화**.
- **Golden:** stdout 전체 diff로 **포맷·수치 동시 회귀**.

### 5.5 클린코드·리팩토링 — 장점과 어려운 점

| 장점 | 어려운 점 |
|------|-----------|
| baseline/Golden으로 **대규모 치환 후에도 안심** | 초기 baseline 확보·갱신 사유 관리 부담 |
| 모듈 분리 후 **테스트·리뷰 단위 축소** | Phase마다 Green 유지하는 **커밋·브랜치 디스플린** 필요 |
| enum·테이블로 **도메인 언어가 코드에 반영** | 2차 리팩토링 범위가 커져 **문서·TC 수 동기화** 필요 |

---

## 6. TC 작성 팁 & 다음 프로젝트 Best Practice 5가지

### 6.1 TC 작성 팁 (본 프로젝트에서 검증된 것)

1. **경계값을 ID로 고정** — `TP-P2-07`(BMI=25)처럼 요구 §3.3 항목마다 TEST 하나.
2. **Given-When-Then 주석** — AI·리뷰어가 의도를 바로 파악.
3. **단일 기대 per TEST** — 실패 시 원인 분리 (gtest 필터 `ctest -R` 활용).
4. **Friend peer + 픽스처 병행** — P0/P2는 순수 함수 직접, P1/P3는 최소 CSV.
5. **결함 ID ↔ TEST 이름 매핑** — `defect_list` §12와 `test_plan` §12를 동시 갱신.

### 6.2 다음 프로젝트 Best Practice 5가지

| # | Practice | 설명 |
|---|----------|------|
| **1** | **수정 워크플로우 문서화** | “리팩토링 → 기능 → 테스트 계획 → TDD → Golden” 순서를 README·`.cursorrules`에 명시하고, **단계별 완료 조건**(빌드 only vs ctest Green)을 분리한다. |
| **2** | **Baseline 이중화** | 수동 `refactor_baseline_output.txt` + **approved golden 파일**로 포맷·수치 회귀를 분리한다. |
| **3** | **경계·결함 매트릭스 선행** | 코딩 전 `requirements` §경계 + `defect_report` §4.5 표를 만들고, TC-ID를 **미리** 배정한다. |
| **4** | **정적(I/O)·동적(도메인) 결함 채널 분리** | CSV·파싱·상한은 6단계 체크리스트, BMI·집계는 8단계 TDD — 본 프로젝트의 **62.5% / 37.5%** 비율이 참고 모델이다. |
| **5** | **커버리지는 “모듈 합산”으로 재정의** | 파사드만 90%가 아니라 `shealth_lib` 전체 + CI에서 `SHEALTH_COVERAGE`를 **12단계 게이트**로 둔다. |

---

## 7. 미완·후속 권고

| 항목 | 권고 |
|------|------|
| 커버리지 회귀 | PR·릴리스 전 `cmake --build build --target coverage` |
| README Activities 5 체크박스 | 본 문서 반영 후 README `[x]` 처리 (선택) |
| 80대 연령·5단계 BMI | 요구 범위 밖 — 확장 시 `refactoring_plan` OCP 항목 참고 |

---

## 부록 A. 테스트 구성 (56건 분류)

| 분류 | 수 | 예시 |
|------|-----|------|
| P0 BMI 계산 | 5 | `ComputeBmi_StandardWeightHeight_*` |
| P2 분류 | 8 | `ClassifyBmi_25_ReturnsObesity` |
| P1 weight 보정 | 6 | `ImputeWeight_SingleZeroIn20s_*` |
| P1 height 보정 | 3 | `ImputeHeight_*`, `CalculateBmi_HeightZeroAfterImpute_*` |
| P3 연령·통계·파일 | 17 | `GetBmiRatio_*`, `CalculateBmi_TooFewColumns_*` |
| Golden Master | 2 | `GoldenMaster_ShealthDat_*` |
| F-04 정상 목록 | 4 | `GetNormalBmiUserIds_*` |
| F-05 전체 비율 | 3 | `GetOverallBmiRatio_*` |
| DEF-007 cap | 2 | `CalculateBmi_ExceedsMaxRecords_*` |
| CsvLoader 단위 | 3 | `CsvLoaderTest.*` |
| 리포트 포맷 | 2 | `Report_Format_*` |
| **합계** | **56** | |

---

## 부록 B. 기준 출력 (회귀 Oracle)

`docs/refactor_baseline_output.txt` / Golden approved (6연령×4분류):

```
20 - underweight = 3.511053, normal = 23.797139, overweight = 11.833550, obesity = 60.858257
30 - underweight = 1.863354, normal = 15.527950, overweight = 10.062112, obesity = 72.546584
40 - underweight = 0.521512, normal = 10.039113, overweight = 9.126467, obesity = 80.312907
50 - underweight = 2.181401, normal = 12.629162, overweight = 9.988519, obesity = 75.200918
60 - underweight = 0.862895, normal = 8.533078, overweight = 10.642378, obesity = 79.961649
70 - underweight = 0.529101, normal = 12.345679, overweight = 10.758377, obesity = 76.366843
```

---

*문서 버전: 1.1 | 워크플로우 12단계 | ctest: **56/56 Green** | coverage: **99.0% / 97.5%** | 인벤토리: `docs/defect_list.md`*
