# Test Operations Guide

## Purpose
- Keep regression safety while refactoring.
- Make `unit`, `smoke`, and `full` runs consistent between local and CI.

## Test Layers
- `unit`: fast characterization and unit tests under `test/unit/`.
- `smoke`: small representative E2E subset for quick sanity.
- `full`: complete `ctest` suite.

## Local Commands
- Configure and build:
```sh
cmake -S . -B build
cmake --build build -j4
```
- Unit only:
```sh
ctest --output-on-failure -L unit -j4 --test-dir build
```
- Smoke only:
```sh
ctest --output-on-failure -L smoke -j4 --test-dir build
```
- Full:
```sh
ctest --output-on-failure -j4 --test-dir build
```

## Execution Rule
- Run in this order when changing core code:
  1. `unit`
  2. `smoke`
  3. `full`
- Do not run `smoke` and `full` in parallel in the same build directory.

## Known Pitfall
- `te_ac_hubbard_square` can fail intermittently when `smoke` and `full` are launched concurrently from the same build tree.
- Recommended mitigation:
  - run `unit -> smoke -> full` sequentially in one shell session
  - if `te_ac_hubbard_square` fails once, rerun it in isolation before judging regression
- Isolation command:
```sh
ctest --output-on-failure -R '^te_ac_hubbard_square$' --test-dir build
```

## Fixtures and Dependencies
- Time-evolution restart-vector tests are chained by fixtures in `test/CMakeLists.txt`.
- `te_dc_hubbard_square`, `te_pulse_hubbard_square`, and `te_quench_hubbard_square` require `te_ac_hubbard_square`.

## CI Policy
- Pull requests and pushes:
  - Run `ctest -L unit`
  - Run `ctest -L smoke`
- Scheduled runs:
  - Run full `ctest` on partitioned matrix jobs.

## Adding a New Unit Test
- Add a script under `test/unit/` with:
  - `set -eu`
  - isolated `WORKDIR`
  - explicit PASS/FAIL checks (`test`, `grep`, `awk`)
- Register it in `test/unit/CMakeLists.txt` with label `unit`.
- Keep runtime short and deterministic.

## Refactoring Guardrails
- Use test-first flow for refactoring:
  1. Add/extend tests first (commit A)
  2. Refactor implementation after tests are green (commit B)
- For core paths, include at least one check each for:
  - normal case
  - boundary case
  - invalid/error case

## Behavior-Equivalence Check (Old vs New)
- For risky refactors, compare baseline/candidate binaries on shared inputs.
- InterAll-focused helper (random fixtures):
```sh
python3 test/tools/compare_readdef_interall_versions.py \
  --baseline /path/to/baseline/HPhi \
  --candidate /path/to/candidate/HPhi \
  --trials 20 \
  --seed 20260208 \
  --tol 1e-10
```
- sz-focused helper (random standard-mode fixtures):
```sh
python3 test/tools/compare_sz_versions.py \
  --baseline /path/to/baseline/HPhi \
  --candidate /path/to/candidate/HPhi \
  --trials 30 \
  --seed 20260208
```
- On mismatch, keep the generated trial artifacts and investigate before merging.

## Log Rule for Refactoring Work
- Update `refactoring_log/work_log.md` after each completed step.
- Add a session file in `refactoring_log/` with date/time at the top.
