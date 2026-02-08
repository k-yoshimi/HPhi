Date: 2026-02-06
Time: 23:27:08 JST

# Refactoring Session 033 (REF-009 start)

## 実施内容
- `src/readdef.c` の `ReadDefFileNInt` に対して switch 分岐の整理を実施。
- 追加した構造:
  - `ReadNIntContext`
  - `ReadNIntDispatchEntry`
  - `DispatchReadNIntKeyword`
- 追加した主な handler:
  - `HandleNIntCalcMod`
  - `HandleNIntModPara`
  - `HandleNIntTEOneBody`
  - `HandleNIntTETwoBody`
  - `HandleNIntBoost`
  - その他 count 読み取り系 handler（LocSpin/Trans/InterAll など）
- `ReadDefFileNInt` 本体は `DispatchReadNIntKeyword` 呼び出し中心の構造へ置換。

## 目的への効果
- `ReadDefFileNInt` 本体の見通しを改善し、分岐ごとの責務を局所化。
- 次段の `ReadDefFileIdxPara` 分割へつながる土台を作成。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (10/10)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (70/70)
