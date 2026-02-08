Date: 2026-02-07
Time: 00:02:40 JST

# Refactoring Session 038 (Task order 1 -> 5 completion)

## 実施内容
- `test/unit/CMakeLists.txt` に新規 characterization test を登録。
  - `unit_sz_kondogc_characterization`
  - `unit_diagonalcalc_kondo_characterization`
- REF-013 (`doc/test_operations.md`) の追加状態を確認し、運用ドキュメントの導入を完了扱いへ更新。
- REF-009 (`src/readdef.c`) の `ReadDefFileNInt` 末尾処理 helper 分割を反映状態として確認。
- REF-012 (`src/sz.c`, `src/diagonalcalc.c`) の追加 helper 分割と Kondo 系 characterization 2件を確定。
- REF-006 (`src/bitcalc.c`) の helper 化（`GetNsiteMultiplierByModel`）を完了扱いで記録。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (14/14)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (74/74)

## 備考
- `unit -> smoke -> full` を逐次実行し、同一buildディレクトリでのテスト競合を回避した。
