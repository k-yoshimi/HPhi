Date: 2026-02-07
Time: 00:41:14 JST

# Refactoring Session 045 (diagonalcalc intra cleanup + KondoNConserved test)

## 実施内容
- `src/diagonalcalc.c` の intra one-body 重複を追加整理。
  - `ApplyTEChemiIntraProcess` で model分岐を簡素化。
  - `Spin` + general-spin のみ `AccumulateTEChemiSpinGeneral` で専用処理を保持。
  - それ以外は `ApplyTETransferIntraProcess` を再利用して共通化。
- `test/unit` に KondoNConserved characterization を追加。
  - 新規: `sz_kondo_nconserved_characterization.sh`
  - 追加登録: `unit_sz_kondo_nconserved_characterization` (`test/unit/CMakeLists.txt`)

## 挙動維持
- `Spin` general-spin における TEChemi の `if (num1 != 0)` 条件付き加算は維持。
- それ以外の model は既存 TETransfer intra の計算式を再利用し、重複のみ削減。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (17/17)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (77/77)

## 補足
- `smoke` と `all` の同時実行時は `te_ac_hubbard_square` が干渉で失敗し得るため、
  最終確認は `te_ac` 単体 -> `smoke` -> `all` を逐次実行した。
