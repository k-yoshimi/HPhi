Date: 2026-02-07
Time: 01:10:52 JST

# Refactoring Session 051 (task 1+2 completion)

## 実施内容
- `1` (`readdef.c` duplicate integration):
  - `GetDiagonalInterAll` と `GetDiagonalInterAll_simple` の重複していた diagonal 判定/格納処理を共通 helper に統合。
    - `InterAllTerm`
    - `LoadInterAllTermFromRow`
    - `AppendDiagonalInterAllTerm`
  - `GetDiagonalInterAll_simple` の off-diagonal コピーを `CopyInterAllRow8` 再利用に変更。
- `2` (CI 重複 setup/build の共通化):
  - 新規 composite action:
    - `.github/actions/bootstrap-build/action.yml`
  - `.github/workflows/main.yml` の2 jobで重複していた
    - apt/brew
    - python setup
    - pip install
    - cmake configure/build
    を composite action 呼び出しに置換。

## 検証結果
- `ruby -e 'require "yaml"; YAML.load_file(".github/workflows/main.yml"); YAML.load_file(".github/actions/bootstrap-build/action.yml")'`: PASS
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (20/20)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (80/80)
