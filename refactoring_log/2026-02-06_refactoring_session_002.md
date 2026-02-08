Date: 2026-02-06
Time: 20:55:16 JST

# Refactoring Session 002

## Scope
- Continued plan execution:
  - REF-004: unit test infrastructure
  - REF-005: first unit tests (`bitcalc`)

## Code Changes
- Updated `test/CMakeLists.txt`:
  - Added `add_subdirectory(unit)` for dedicated unit tests.

- Added `test/unit/CMakeLists.txt`:
  - Introduced `unit_bitcalc` executable.
  - Added CTest entry `unit_bitcalc` with label `unit`.
  - Linked required source files:
    - `src/bitcalc.c`
    - `src/ErrorMessage.c`
    - `src/global.c`
  - Added compile definition `DSFMT_MEXP=19937`.

- Added `test/unit/test_bitcalc.c`:
  - Minimal self-contained C unit test runner.
  - Covered:
    - `GetSplitBit`
    - `GetSplitBitByModel`
    - `SplitBit`
    - `GetOffComp`
    - `GetSplitBitForGeneralSpin`
    - `snoob`
    - `pop`
  - Includes normal, boundary, and invalid-input checks.

## Verification
- `ctest -N -L unit` => 1 test (`unit_bitcalc`)
- `ctest --output-on-failure -L unit` => pass
- `ctest --output-on-failure -L smoke -j4` => pass
- `ctest --output-on-failure -j4` => 61/61 passed

## Notes
- During implementation, initial link errors occurred for `unit_bitcalc` because `bitcalc.c` references globals defined in `global.c`; resolved by linking `src/global.c`.
