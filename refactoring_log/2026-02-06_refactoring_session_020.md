Date: 2026-02-06
Time: 22:37:17 JST

# Refactoring Session 020 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalCoulombIntra` を小分け。
- 追加 helper:
  - `AddDiagonalConstantTerm`
  - `AddDiagonalMaskedBySequentialIndex`
  - `AddDiagonalMaskedByRestrictedBasis`
- `SetDiagonalCoulombIntra` は、model 判定と mask 計算を残しつつ helper 呼び出し構造へ整理。

## 挙動維持の要点
- `isite1 > Nsite`（inter-process）と `isite1 <= Nsite`（intra-process）の分岐は維持。
- `HubbardGC` が `(j-1)&mask` を使う挙動と、他モデルが `list_1[j]&mask` を使う挙動を維持。
- `Spin/SpinGC` で Coulomb 項を無視する挙動を維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
