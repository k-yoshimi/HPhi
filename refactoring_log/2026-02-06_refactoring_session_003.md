Date: 2026-02-06
Time: 20:58:27 JST

# Refactoring Session 003

## Scope
- Continued plan execution:
  - REF-006: behavior-preserving refactor in `bitcalc`
  - REF-007: add unit tests for `common/setmemory`

## Code Changes
- Updated `src/bitcalc.c`:
  - Added internal helper `GetNsiteMultiplierByModel`.
  - Refactored `GetSplitBitByModel` to use the helper and remove model-switch duplication.
  - Behavior and error handling kept unchanged.

- Updated `test/unit/CMakeLists.txt`:
  - Added `unit_setmemory` executable and CTest registration with `unit` label.

- Added `test/unit/test_setmemory.c`:
  - Tests for contiguous allocation/zero initialization and indexing:
    - `i_2d_allocate`, `i_3d_allocate`
    - `ui_1d_allocate`, `li_1d_allocate`, `lui_1d_allocate`, `d_1d_allocate`, `cd_1d_allocate`
    - `d_2d_allocate`, `cd_3d_allocate`
  - Includes free-function calls for each tested allocation type.

## Verification
- `ctest -N -L unit` => 2 tests (`unit_bitcalc`, `unit_setmemory`)
- `ctest --output-on-failure -L unit` => pass
- `ctest --output-on-failure -L smoke -j4` => pass
- `ctest --output-on-failure -j4` => 62/62 passed

## Notes
- A transient `Not Run` occurred once for `unit_setmemory` when build and ctest were launched in parallel. Re-running ctest after build completion passed without issues.
