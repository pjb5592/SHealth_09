# SHealth BMI 프로젝트 — 최적화 프롬프트 (2026-05-20)

> 원본: `README.md`, `TDD 기반 암호 검사기 또는 BMI 프로젝트용 프롬프트.txt`, `src/main/cpp/SHealth.*`  
> 목적: 대화 이력·중복·모호 표현을 제거하고, **단계별 재실행 가능한 프롬프트**로 정리

---

## 사용 방법

1. **공통 컨텍스트**를 먼저 읽고, 해당 단계 프롬프트를 그대로 복사해 사용한다.
2. 단계는 **의존 순서**를 따른다 (이전 단계 산출물 `@` 참조).
3. 코드·구현 변경 단계는 반드시 `cmake --build build && ctest` Green을 완료 조건으로 둔다.
4. 세션 종료 시 **후처리 프롬프트**를 한 번 실행한다.

---

## 공통 컨텍스트 (모든 단계에 적용)

| 항목 | 내용 |
|------|------|
| 프로젝트 | SHealth BMI — C++17, CMake 3.10+, Google Test (FetchContent 자동 다운로드) |
| 도메인 | CSV(`shealth.dat`)에서 ID·나이·체중(kg)·키(cm) 로드 → BMI 계산 → 연령대별 체중 분포 통계 |
| BMI 공식 | `체중(kg) / (키(m))²` — 키는 cm 입력, 계산 시 `/100.0` 변환 |
| BMI 분류 | ≤18.5 저체중 · 18.5 초과 23 미만 정상 · 23 이상 25 미만 과체중 · 25 이상 비만 |
| 연령대 | 20·30·40·50·60·70대 — 각 `[a, a+10)` 구간 (예: 20대 = 20≤나이<30) |
| 누락 보정 | 체중 `0.0` → 동일 연령대 유효 체중 평균으로 대체 (README 기준) |
| API 관례 | `getBmiRatio(ageClass, type)` — ageClass: 20/30/…/70, type: 100=저체중·200=정상·300=과체중·400=비만 |
| 테스트 | Given-When-Then · `TEST`/`TEST_F` · 경계값(18.5, 23, 25, 연령 경계 20/30/…) 필수 |
| 리팩토링 | 테스트 Green 상태에서만 · 동작 변경 없음 · 매직 넘버 상수화 |
| 검증 명령 | `mkdir build && cd build && cmake .. && cmake --build . && ctest` |
| 산출물 언어 | 설명·문서는 **한글**, 코드·식별자는 기존 프로젝트 규칙 준수 |

**프롬프트 구조:** `[P]` 역할 · `[C]` 맥락 · `[T]` 작업 · `[F]` 산출물 형식

**주요 소스 경로**

| 역할 | 경로 |
|------|------|
| 헤더 | `src/main/cpp/SHealth.h` |
| 구현 | `src/main/cpp/SHealth.cpp` |
| 실행 진입점 | `src/main/cpp/SHealthBMI.cpp` |
| 단위 테스트 | `src/test/cpp/SHealthBMITest.cpp` |
| 샘플 데이터 | `shealth.dat` |
| 빌드 | `CMakeLists.txt` |

---

## 단계별 프롬프트

### 0. 프로젝트 규칙 (.cursorrules)

```
[P] 레거시 C++ QA/리팩토링 시니어 엔지니어
[C] SHealth BMI C++17 프로젝트 — Cursor AI가 항상 따를 프로젝트 규칙 정의
[T] 프로젝트 루트 `.cursorrules` 작성
    - 기술 스택: C++17, CMake, Google Test, shealth_lib
    - 도메인: BMI 공식, 4단계 분류 기준, 연령대 구간, 체중 0 평균 보정
    - 테스트: Given-When-Then, 경계값(18.5/23/25, 연령 20·30·…·70), 단일 조건 per TEST
    - TDD: Red → Green → Refactor, 실패 테스트 우선
    - 리팩토링: Green에서만, 매직 넘버(100/200/300/400, 18.5/23/25) 상수화, 최소 변경
    - README Activities 1~5 순서 존중
[F] `.cursorrules`에 붙여넣기 가능한 완성 텍스트 (한글 설명)
```

**완료 기준:** `.cursorrules` 파일 생성

---

### 1. 요구사항 분석

```
@README.md @bmi.png

[P] 시니어 C++ QA 엔지니어
[C] SHealth BMI C++17 (CMake, Google Test) — README 기반 도메인 요구사항
[T] C++ 구현·테스트 관점으로 요구사항 재정리
    1) 기본 원칙 및 범위 (삼성 헬스 연령대별 BMI 통계)
    2) 입력 데이터 형식 (shealth.dat CSV) — 표
    3) BMI 계산 및 4단계 분류 기준 — 표
    4) 체중 누락(0) 연령대 평균 보정 — 표
    5) 연령대별 저체중/정상/과체중/비만 비율 산출 — 표
    6) getBmiRatio(ageClass, type) API 계약 — 표
    7) README Activities 4 기능 개선 항목 (SRP 분리, 키 0 보정, 정상 BMI 목록, 전체 비율 등)
    8) Google Test 시나리오 번호 목록 (30건 이상 권장)
[F] Markdown(표+번호 목록) → docs/requirements_analysis.md 저장
```

**완료 기준:** `docs/requirements_analysis.md` 생성 (코드 변경 없음)

---

### 2. 코드 품질 분석

```
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp @src/main/cpp/SHealthBMI.cpp

[P] 시니어 C++ 아키텍트 + 모던 C++ 리뷰어
[C] SHealth 클래스 — SOLID·Code Smell 관점 정적 분석 (현재 레거시 구현)
[T] SHealth 클래스 품질 분석
    - SRP/OCP/LSP/ISP/DIP 위반 여부
    - Magic Number (18.5, 23, 25, 100/200/300/400, 고정 배열 10000)
    - 중복 분기 (연령대별 if-else, getBmiRatio 스위치)
    - 데이터 로딩·보정·BMI 계산·통계 집계의 결합도
    - 경계값 버그 후보 (BMI=18.5/23/25, sum=0, ageCount=0)
    - 리팩토링 우선순위 1~5 (근거 포함)
[F] Markdown 표 → docs/code_quality_report.md 저장
```

**완료 기준:** `docs/code_quality_report.md` 생성

---

### 3. 테스트 계획

```
@src/main/cpp/SHealth.h @README.md @docs/requirements_analysis.md

[P] 시니어 QA 리드
[C] 요구사항 분석 결과 기반 단위·회귀 테스트 전략
[T] 테스트 계획서 작성
    - TDD 4영역 우선순위 (P0/P1/P2)
      1) BMI 계산 로직
      2) 연령대 평균 보정 (weight=0, 추후 height=0)
      3) BMI 카테고리 분류
      4) 연령대별 BMI 통계·getBmiRatio
    - 경계값 매트릭스 (18.5, 23, 25, 연령 19/20/29/30, 빈 파일, 잘못된 CSV)
    - 예외·특이 케이스 (sum=0, 전원 체중 0, 파일 미존재)
    - Fake/Fixture 데이터 전략 (소형 in-memory CSV 또는 임시 파일)
    - 커버리지 목표(90%+) 및 측정·개선 절차
[F] docs/test_plan.md 저장
```

**완료 기준:** `docs/test_plan.md` 생성

---

### 4. 테스트 구현 (TDD)

```
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp
@src/test/cpp/SHealthBMITest.cpp @shealth.dat
@docs/requirements_analysis.md @docs/test_plan.md

[P] 테스트 설계에 강한 시니어 C++ QA
[C] C++17, Google Test — 현재 SHealthBMITest는 FAIL() 플레이스홀더
[T] TDD Red → Green
    1) 먼저 실패하는 테스트 작성 (단일 조건, 명확한 TEST 이름)
       - BMI 계산
       - weight=0 평균 보정
       - 4단계 분류
       - 연령대별 비율·getBmiRatio
    2) 테스트 통과에 필요한 최소 구현만 SHealth에 추가·수정
    3) Given-When-Then 주석, EXPECT_NEAR/EXPECT_DOUBLE_EQ 적절 사용
[F] 완성 테스트 코드 + 구현 diff. cmake --build build && ctest Green
```

**완료 기준:** `ctest` 전체 Green (`SHealthBMITest` 포함)

**빠른 TDD 시작 (단일 프롬프트)**

```
BMI 계산기(SHealth)에 대해 TDD를 시작하고 싶어.
1. BMI 계산 로직 테스트
2. 평균 보정(weight=0) 테스트
3. BMI 카테고리 분류 테스트
4. 연령대별 BMI 통계·getBmiRatio 테스트
위 4가지에 대해 먼저 실패하는 Google Test 코드를 작성해줘.
각 테스트는 단일 조건을 검사하고, 이름은 명확하게 지어줘.
```

---

### 5. 1차 리팩토링 (클린코드)

```
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp
@src/test/cpp/SHealthBMITest.cpp @docs/code_quality_report.md

[P] 모던 C++ 리팩토링 코치
[C] ctest Green 기준선 — README Activities 2 (클린코드 1차)
[T] 동작 변경 없이 리팩토링
    - 네이밍 개선 (underweight20 등 → 구조체/맵/enum)
    - 하드코드·매직 넘버 상수화 (18.5, 23, 25, 100/200/300/400)
    - 함수 추출 (CSV 파싱, 보정, BMI 계산, 연령대 통계)
    - 반복/중복 제거 (연령대별 if-else, getBmiRatio 분기)
[F] 리팩토링 diff + ctest Green 유지
```

**완료 기준:** `ctest` Green + 가독성·중복 개선

**클린코드 전용 프롬프트**

```
현재 SHealth C++ 코드를 클린코드 원칙(SRP, DRY, 의미 있는 네이밍, 함수 분리)에 따라 리팩토링해줘.
특히 Magic Number 제거, 한 함수의 책임 최소화, 테스트하기 쉬운 분리에 주의해줘.
테스트는 Green을 유지해줘.
```

---

### 6. 결함 분석·문서화

```
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp
@src/test/cpp/SHealthBMITest.cpp @shealth.dat
@docs/requirements_analysis.md

[P] C++ QA 엔지니어 (디버깅·결함 분석)
[C] README BMI 기준 vs 구현 분기 (`<=`, `>`, `>=` 경계) 불일치 가능성
[T]
    A) ctest 실패·요구사항 불일치·경계값 분석
       - BMI 18.5/23/25 경계, 연령 구간, 0 나눗셈
       - 버그 위치, 심각도, 최소 수정안
    B) 결함 목록 문서화
       - ID, Severity, ItemType, Steps, Expected, Actual, Root Cause, Fix Summary
[F]
    A) 수정 diff + ctest Green 확인
    B) docs/defect_list.md (DEF-001~ 형식)
```

**완료 기준:** `docs/defect_list.md` + `ctest` Green

---

### 7. 기능 개선 (SRP·확장)

```
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp
@docs/requirements_analysis.md @docs/test_plan.md
@src/test/cpp/SHealthBMITest.cpp

[P] 시니어 C++ 개발자
[C] README Activities 4 — 테스트 선행(TDD) 후 구현
[T] 기능별 테스트 추가 후 구현
    - SRP에 따른 책임 분리 (파서, 보정, 분류, 통계)
    - 특정 연령대 BMI 분포 비율 조회 API
    - Height=0 연령대 평균 보정 (weight=0과 대칭)
    - BMI 정상 범위 사용자 목록 조회
    - 전체 사용자 대비 각 BMI 범주 비율
[F] 테스트 + 구현 + docs/feature_changelog.md (선택)
```

**완료 기준:** 신규 기능별 테스트 Green

---

### 8. Golden Master / 회귀 테스트 (선택)

```
@src/main/cpp/SHealthBMI.cpp @shealth.dat
@src/test/cpp/SHealthBMITest.cpp

[P] Approval/Golden Master 회귀 테스트 설계 전문가
[C] SHealthBMI main 출력 — 연령대×4분류 비율 printf 결과
[T] Golden Master 체계 구현
    1) `SHealthBMI` 실행 기대 출력 캡처 (test/golden/shealth_bmi.approved.txt)
    2) Google Test로 계산 결과 vs golden 비교 (또는 getBmiRatio 일괄 검증)
    3) CMake/ctest 통합 + golden 갱신 방법 문서화
[F] 테스트 코드 + 실행·갱신 방법 (README 섹션 또는 주석)
```

**완료 기준:** Golden/회귀 테스트 포함 `ctest` Green

---

### 9. 리팩토링 로드맵 (2차·구조)

```
@docs/code_quality_report.md @docs/requirements_analysis.md
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp

[P] 모던 C++ 리팩토링 코치
[C] 1차 리팩토링·기능 개선 후 ctest Green 기준선
[T] SHealth 2차 구조 개선 로드맵
    - 고정 배열(10000) → STL 컨테이너
    - 연령대·분류 enum/struct 테이블 드리븐 설계
    - getBmiRatio 분기 → 맵 또는 계산 함수
    - Phase별: 목표 / 변경 파일 / 리스크 / 롤백 / 검증 명령
[F] docs/refactoring_plan.md (Phase 0~N 체크리스트)
    각 Phase 완료 시: cmake --build build && ctest
```

**완료 기준:** 실행 가능한 Phase 로드맵 문서

---

### 10. 결함 관리 프로세스 (QA)

```
@docs/defect_list.md @docs/requirements_analysis.md @docs/test_plan.md

[P] QA 리드 엔지니어
[C] defect_list(인벤토리)와 분리된 **프로세스·템플릿** 문서
[T] 결함 관리 체계 문서 작성
    1) Severity × ItemType(5종) 분류 매트릭스
    2) 결함 보고서 템플릿 (재현/기대/실제/원인/수정/검증)
    3) 품질 메트릭 (통과율, 커버리지, 경계값 발견율)
    4) (선택) GitHub Issues 연동 워크플로
[F] docs/defect_report.md 저장
```

**완료 기준:** `docs/defect_report.md` 생성

---

### 11. 설계 다이어그램 (선택)

```
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp
@src/test/cpp/SHealthBMITest.cpp

[P] 소프트웨어 아키텍트
[C] SHealth BMI — 도메인·테스트 관계 시각화
[T] 클래스 다이어그램 작성
    - SHealth, (리팩토링 후) 파서·보정·분류·통계 컴포넌트
    - 주요 public API, 테스트 Fixture 관계
[F] Mermaid classDiagram → docs/architecture.md
```

**완료 기준:** `docs/architecture.md` 생성

---

### 12. QA 종합 검토 (최종)

```
@docs/requirements_analysis.md @docs/code_quality_report.md
@docs/test_plan.md @docs/defect_report.md @docs/defect_list.md
@src/main/cpp/SHealth.h @src/main/cpp/SHealth.cpp
@src/test/cpp/SHealthBMITest.cpp

[P] QA 리드 엔지니어
[C] 1~11단계 산출물 + 실제 ctest 결과
[T] QA 활동 종합 보고 (README Activities 5 회고 항목 반영)
    1) 테스트 완료율·커버리지 (목표 대비, 측정 명령)
    2) 결함 패턴 (ItemType·Severity 분포)
    3) 코드 품질 Before & After
    4) AI 활용 효과·한계, TC 작성 팁
    5) 다음 레거시 BMI/헬스 데이터 프로젝트 Best Practice 5가지
[F] docs/qa_final_report.md 저장
    작성 전 ctest 재검증 후 수치 반영
```

**완료 기준:** `docs/qa_final_report.md` + 최신 ctest 수치 반영

---

## 후처리 프롬프트 (각 단계 완료 후 1회)

```
[P] 프로젝트 문서·배포 담당
[C] 방금 완료한 단계 번호 N, 산출물 경로
[T] 아래 순서로 실행
    1) Report/N.<단계-slug>-report-YYYY-MM-DD.md 생성
       - 수행 작업, 변경 파일, 검증 결과(ctest), 다음 단계 권고
    2) Prompt/N.<단계-slug>-transcript-YYYY-MM-DD_prompt.md Export
       - User 프롬프트 + Assistant 요약만 (대화형 형식)
    3) Prompt/full-transcript-YYYY-MM-DD_prompt.md 갱신 (세션 N 반영)
    4) (사용자 요청 시) git add → commit → push
[F] 생성·갱신된 파일 경로 목록 + git 결과
```

**슬러그 예:** `requirements-analysis`, `tdd-implementation`, `clean-refactor`, `feature-enhancement`, `qa-final-report`

---

## 단계 의존 관계

```mermaid
flowchart TD
    S0[0.cursorrules] --> S1[1.요구사항]
    S1 --> S2[2.코드품질]
    S1 --> S3[3.테스트계획]
    S3 --> S4[4.TDD구현]
    S4 --> S5[5.1차리팩토링]
    S4 --> S6[6.결함분석]
    S4 --> S7[7.기능개선]
    S4 --> S8[8.Golden Master]
    S2 --> S5
    S5 --> S9[9.2차리팩토링로드맵]
    S7 --> S9
    S6 --> S10[10.결함관리]
    S1 --> S10
    S3 --> S10
    S4 --> S11[11.설계다이어그램]
    S1 --> S12[12.QA종합]
    S2 --> S12
    S3 --> S12
    S6 --> S12
    S7 --> S12
    S8 --> S12
    S10 --> S12
```

---

## TDD·리팩토링 빠른 참조 (원본 프롬프트 통합)

| # | 상황 | 핵심 지시 |
|---|------|-----------|
| 1 | TDD 시작 | 4영역 실패 테스트 먼저 → 단일 조건·명확한 이름 |
| 2 | Green | 테스트 통과 최소 C++ 구현 (`SHealth` 중심) |
| 3 | 클린코드 | SRP·DRY·Magic Number·함수 분리, Green 유지 |
| 4 | 설계 | 클래스 다이어그램 + 테스트·도메인 관계 |
| 5 | 커버리지 | 분기·경계·예외 누락 TC 제안 |

---

## 원본 대비 최적화 요약

| 개선 항목 | 내용 |
|-----------|------|
| 중복 제거 | BMI 기준·연령대·API 관례·빌드 명령을 **공통 컨텍스트**로 1회만 명시 |
| 프로젝트 특화 | TV/Tuner 대신 `SHealth`, `shealth.dat`, `getBmiRatio` 반영 |
| README 연동 | Activities 1~5를 단계 1~2·4~5·7·12에 매핑 |
| TDD txt 통합 | 4영역 TDD·클린코드·다이어그램·커버리지를 단계 4·5·11·표로 수렴 |
| 의존성 명시 | `@` 참조 파일·선행 docs 고정 |
| 완료 기준 | 각 단계 검증 가능한 **Done** 조건 |
| 후처리 통합 | Report/Transcript/Git을 **단일 후처리 프롬프트**로 통합 |

---

## 빠른 복사: 전체 워크플로우 (한 번에 지시할 때)

```
SHealth BMI C++17 TDD/QA 프로젝트를 아래 순서로 진행해줘.
공통 제약: BMI 공식·4분류 기준·연령대 [20,30,…,70) 구간, weight=0 평균 보정,
Given-When-Then TEST, Green에서만 리팩토링.

0) .cursorrules 작성
1) @README.md → docs/requirements_analysis.md
2) SHealth 품질 → docs/code_quality_report.md
3) 테스트 계획 → docs/test_plan.md
4) TDD: 4영역 실패 테스트 → 최소 구현 → Green
5) 클린코드 1차 리팩토링 (code_quality_report 기준)
6) 결함 분석·docs/defect_list.md + 수정 후 Green
7) README 기능 개선 (SRP, height=0, 목록·전체 비율) + 테스트
8) (선택) Golden Master — SHealthBMI 출력 또는 getBmiRatio 회귀
9) 2차 리팩토링 로드맵 → docs/refactoring_plan.md
10) docs/defect_report.md (프로세스·템플릿)
11) (선택) docs/architecture.md 클래스 다이어그램
12) docs/qa_final_report.md (ctest 재검증 후 작성)

각 단계 완료 시: Report + Prompt transcript Export + full-transcript 갱신.
모든 설명은 한글, 검증은 cmake --build build && ctest.
```
