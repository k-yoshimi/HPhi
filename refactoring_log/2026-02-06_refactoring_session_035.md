Date: 2026-02-06
Time: 23:41:29 JST

# Refactoring Session 035 (REF-009 continuation)

## 実施内容
- `ReadDefFileIdxPara` の残存大型ケースを helper 関数へ分割。
- 追加 helper:
  - `ParseIdxInterAllDef`
  - `ParseIdxThreeBodyGDef`
  - `ParseIdxFourBodyGDef`
  - `ParseIdxSixBodyGDef`
  - `ParseIdxLaserDef`
  - `ParseIdxTEOneBodyDef`
  - `ParseIdxTETwoBodyDef`
  - `ParseIdxBoostDef`
  - `ParseIdxSingleExcitationDef`
  - `ParseIdxPairExcitationDef`
- `ReadDefFileIdxPara` の `KWInterAll` から `KWPairExcitation` まで、case 本体を helper 呼び出しに置換。
- `ReadDefFileIdxPara` 内の不要なローカル変数を削除し、可読性を改善。

## 挙動維持
- 既存の model 判定、site/pair/quad 検証、Hermite 検証、`InterAll`/`TEInterAll` の整理手順を維持。
- `SingleExcitation` の spin 系禁止条件と `PairExcitation` の `itype` に応じた符号処理を維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (10/10)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (70/70)

## 補足
- 途中で `ctest -L smoke` と `ctest` 全体を同時実行したため一時的に失敗が発生。
- 同一変更で逐次実行に切り替えると再現せず、すべて PASS を確認。
