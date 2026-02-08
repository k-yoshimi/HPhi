Date: 2026-02-06
Time: 22:01:19 JST

# Refactoring Session 012 (REF-012 partial)

## 実施内容
- `src/sz.c` の `Hubbard` 系ケース分岐の重複を helper 化。
- 追加した helper:
  - `ComputeSzCountForHubbardFamily`
- `sz()` の `switch (X->Def.iCalcModel)` で `Hubbard / HubbardNConserved` を共通実装へ統合。
- `hacker` 値 (`0/1`) ごとの分岐とエラーメッセージは既存挙動を維持。
- `Hubbard` (`hacker=0` は serial) と `HubbardNConserved` (`hacker=0` は parallel) の実行形態の差も維持。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)
