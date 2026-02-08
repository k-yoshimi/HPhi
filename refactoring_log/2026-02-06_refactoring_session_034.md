Date: 2026-02-06
Time: 23:31:56 JST

# Refactoring Session 034 (REF-009 continuation)

## 実施内容
- `ReadDefFileIdxPara` の可読性改善として、主要ケースを helper に分割。
- 追加 helper:
  - `ParseIdxLocSpinDef`
  - `ParseIdxTransferDef`
  - `ParseIdxCoulombIntraDef`
  - `ParseIdxCoulombInterDef`
  - `ParseIdxHundDef`
  - `ParseIdxPairHopDef`
  - `ParseIdxExchangeDef`
  - `ParseIdxIsingDef`
  - `ParseIdxPairLiftDef`
  - `ParseIdxOneBodyGDef`
  - `ParseIdxTwoBodyGDef`
- `ReadDefFileIdxPara` の switch では上記 helper 呼び出しのみを残し、本体の見通しを改善。

## 挙動維持
- site/pair/quad 検証、Hermite 検証、model制約の判定を変更せず移設。
- `PairHop` の対称登録と `Ising` の `Hund/CoulombInter` 反映ロジックを維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (10/10)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (70/70)
