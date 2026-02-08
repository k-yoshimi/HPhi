Date: 2026-02-06
Time: 22:09:30 JST

# Refactoring Session 013 (REF-012 partial)

## 実施内容
- `src/sz.c` の `Kondo` 系ケース分岐の重複を helper 化。
- 追加した helper:
  - `ComputeSzCountForKondoFamily`
- `sz()` の `switch (X->Def.iCalcModel)` で `KondoGC / KondoNConserved / Kondo` を共通実装へ統合。
- 以下の既存差分を維持:
  - `KondoGC` は `count_localized_spins` を先に評価。
  - `KondoNConserved` は `RecordOMPSzMid` + `BarrierMPI` を実行。
  - `Kondo` は `hacker==0/1` のみ計算分岐（既存挙動どおり）。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)
