# SHealth BMI (C++)

# Overview
- 삼성 헬스에서는 수집된 데이터를 활용하여 사용자들의 나이대(ex. 20대, 30대, 40대 등)별 저체중/정상체중/과체중/비만의 통계를 계산하고자 합니다.
- 수집 중 누락된 체중값이 있으며 같은 나이대(ex. 20대, 30대, 40대 등)의 평균을 적용합니다. 체중 0이 누락된 경우 입니다.
- 수집 데이터는 ID, 나이, 몸무게(kg), 키(cm)이며, BMI는 체중(kg) / 키(m)제곱 으로 계산합니다.
- BMI 기준으로 18.5이하 저체중, 18.5초과 23미만 정상체중, 23이상 25미만 과체중, 25이상 비만으로 판단합니다.
![BMI](./bmi.png)
- 제공된 코드에는 다양한 코드 품질 문제가 있습니다. 


## data sample
- 입력 데이터 (shealth.dat)
```
id,age,weight,height
93705,66,79.5,158.3
93708,66,53.5,150.2
93709,75,88.8,151.1
... (이하 생략)
```
- 각 라인별 ID, 나이, 체중(kg), 키(cm) 순서 입니다.


## 빌드 및 실행

### 요구사항
- CMake 3.10 이상
- C++17 지원 컴파일러
- Google Test (CMake에서 자동 다운로드)

### 빌드
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 실행
```bash
./SHealthBMI
```

### 테스트 실행
```bash
cd build
ctest
```

### 코드 커버리지 (gcov / lcov, MinGW GCC)

`SHEALTH_COVERAGE=ON`으로 `shealth_lib`·`SHealthBMITest`를 빌드한 뒤 `coverage` 타깃이 `ctest` → HTML 리포트를 생성한다.

```bash
cmake -B build -DSHEALTH_COVERAGE=ON
cmake --build build
cmake --build build --target coverage
```

| 항목 | 경로 |
|------|------|
| 요약 텍스트 | `build/coverage_summary.txt` (gcov 파싱, **권장**) |
| HTML 리포트 | `build/coverage_html/index.html` (lcov 성공 시; GCC 15에서는 생략될 수 있음) |
| 목표 (test_plan §8.1) | `shealth_lib` .cpp 합산 — 라인 ≥ 90%, 분기 ≥ 85% |

요구: MinGW `gcov`, Python 3. (`lcov`/`genhtml`은 HTML용 선택).

일반 개발 빌드는 `SHEALTH_COVERAGE=OFF`(기본)로 `cmake -B build` 하면 `-O0` 없이 빌드된다.

### Golden Master 회귀 (`shealth.dat` 연령대×4분류 비율)

`SHealthBMI` 실행 결과와 동일한 포맷의 기대 출력을 `test/golden/shealth_bmi.approved.txt`에 보관한다.
Google Test(`GoldenMaster_*`)가 계산 결과와 golden을 비교한다.

| 항목 | 경로 |
|------|------|
| 기대 출력 (approved) | `test/golden/shealth_bmi.approved.txt` |
| 3단계 수동 baseline (참고) | `docs/refactor_baseline_output.txt` |
| 리포트 포맷 (공유) | `src/main/cpp/SHealthBmiReport.cpp` |

**실행**

```bash
cd build
cmake --build .
ctest -R GoldenMaster
```

**golden 갱신** (의도된 동작 변경·버그 수정 후에만)

```bash
cd build
cmake --build . --target update-golden-shealth-bmi
ctest -R GoldenMaster
```

`update-golden-shealth-bmi`는 프로젝트 루트에서 `SHealthBMI`를 실행해 `test/golden/shealth_bmi.approved.txt`를 덮어쓴다.
PR에는 golden diff와 갱신 사유를 함께 기록한다.


## 프로젝트 구조
```
CMakeLists.txt
shealth.dat
src/
  main/cpp/
    SHealth.h          - SHealth 클래스 헤더
    SHealth.cpp        - BMI 계산 및 통계 로직 구현
    SHealthBMI.cpp     - main 함수 (프로그램 진입점)
  test/cpp/
    SHealthBMITest.cpp - Google Test 기반 단위 테스트
test/
  golden/
    shealth_bmi.approved.txt - SHealthBMI Golden Master 기대 출력
```


# 생성형AI를 활용한 Activities (6 시간)
1. 문제 코드 분석 및 코드 스멜 찾기 (1시간)
- [x] 기본 코드구조, BMI 로직 이해 
- [x] 코드 스멜 찾기 
2. 1차 리펙토링 (클린코드 관점, 아래 내용을 순차적으로 수행) (1시간) 
- [x] 네이밍 개선
- [x] 하드코드 및 전역변수 제거 
- [x] 함수 추출
- [x] 반복/중복 제거
3. UnitTest 작성 (1시간)
- [x] BMI 계산 로직 TC
- [x] Age 평균치 보정 로직 TC
- [x] 정상/저체중/과체중/비만 분류 TC
- [x] 예외상황 TC
4. 기능 개선 (2시간)
- [x] SRP에 따른 책임 분리등 리팩토링 
- [x] 특정 연령대의 BMI 분포 비율 계산 기능 추가
- [x] Height가 0인 경우에 대한 평균치 보정 로직 추가 
- [x] BMI 정상 범위 사용자 목록 조회 기능 추가
- [x] 전체 사용자 대비 각 BMI 범주 비율 계산 기능 추가
5. 회고 및 발표 (1시간) 
- [ ] 실습 목표와 달성도 
- [ ] 코드 품질 Before & After
- [ ] AI를 어떻게 활용했나? 도움이 된 순간과 한계는? 
- [ ] TC를 추가해보면서 개선에 미친 영향, TC 작성 팁
- [ ] 클린코드와 리팩토링에서 느낀 장점과 어려운점


# 주의 사항
- 코드 품질을 높이기 위해 C++의 경우 STL을 필요한 경우 사용하셔도 됩니다.
