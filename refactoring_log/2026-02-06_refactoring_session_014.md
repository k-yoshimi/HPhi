Date: 2026-02-06
Time: 22:10:52 JST

# Refactoring Session 014 (REF-012 partial)

## 実施内容
- `src/sz.c` の `Spin` ケース分岐を helper 化。
- 追加した helper:
  - `ComputeSzCountForSpin`
- `sz()` の `case Spin:` は helper 呼び出しに置換。
- 既存挙動の維持点:
  - `iFlgGeneralSpin == FALSE/TRUE` の処理分岐を維持。
  - `hacker == -1/0/1` の処理とエラーメッセージを維持。
  - `hacker == -1` の場合に `icnt` を明示更新しない既存挙動を維持。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)
