Date: 2026-02-07
Time: 12:23:36 JST

# Refactoring Session 055 (InterAll split API unification)

## 背景（`GetDiagonalInterAll_simple` が存在した理由）
- `GetDiagonalInterAll_simple` は 2024-04-18 の導入コミット
  `68ebc474`（`make getdiagonal_simple, CheckInterAllHermite_simple, and ArrangeInterAllOffDiagonal`）
  で追加されていた。
- 目的は、`InterAll` の off-diagonal をその場で model filter 変換せず一旦素通しし、
  後段の `CheckInterAllHermite_simple` と `ArrangeInterAllOffDiagonal` で
  Hermite 性検証と正規化を行う分離フローを取るため。

## 今回の方針
- API を一本化し、挙動差は mode 指定で切り替える。
  - `INTERALL_SPLIT_MODEL_FILTERED`
  - `INTERALL_SPLIT_SIMPLE`
- これにより `GetDiagonalInterAll_simple` の専用公開APIは廃止し、
  呼び出し側は `GetDiagonalInterAll(..., INTERALL_SPLIT_SIMPLE)` を使う。

## 実施内容
- `src/include/readdef.h`
  - `InterAllSplitMode` enum を追加。
  - `GetDiagonalInterAll` に `split_mode` 引数を追加。
  - `GetDiagonalInterAll_simple` 宣言を削除。
- `src/readdef.c`
  - `GetDiagonalInterAll` を mode 受け取りに変更。
  - `split_mode` から `apply_model_filter` を決定する実装に変更。
  - `GetDiagonalInterAll_simple` 実装を削除。
- `src/readdef_idx_parser.c`
  - 2箇所の呼び出しを
    `GetDiagonalInterAll_simple(...)` から
    `GetDiagonalInterAll(..., INTERALL_SPLIT_SIMPLE)` へ移行。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit --parallel 1 --test-dir build`: PASS (20/20)
- `ctest --output-on-failure -L smoke --parallel 1 --test-dir build`: PASS (6/6)

## 備考
- リンク時の `ld: warning: ignoring duplicate libraries: '-lm'` は既存挙動であり、
  今回の変更範囲外。
