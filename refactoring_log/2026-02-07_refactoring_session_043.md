Date: 2026-02-07
Time: 00:32:18 JST

# Refactoring Session 043 (sz Kondo-family cleanup)

## 実施内容
- `src/sz.c` の Kondo 系 `calculate_jb_*` 関数の重複ロジックを helper 化。
  - 新規: `GetKondoSiteOccupations`
  - 新規: `CountKondoRightHalfOccupations`
  - 新規: `GetKondoHalfBitCombinationWindow`
  - 新規: `ComputeKondoFixedParticleContribution`
  - 新規: `HasValidKondoGCLocalizedConfiguration`
- 対象関数:
  - `calculate_jb_Kondo`
  - `calculate_jb_KondoNConserved`
  - `calculate_jb_KondoGC`
- 変更方針:
  - 右半分bitの up/down 抽出と局在スピン制約判定を helper に移動。
  - Kondo 固定粒子数ケースの組合せ寄与計算を helper 化。
  - `count_localized_spins` を Kondo/KondoNConserved 側でも使い、局在サイト数算出の重複を削減。

## 挙動維持
- Kondo 系 `jb` 更新式と局在制約条件は維持。
- 変更は helper 抽出と分岐整理に限定。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (16/16)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (76/76)

## 補足
- 途中で `te_ac_hubbard_square` が1回失敗し、依存3件が `Not Run` になったが、
  単体再実行と再度の `smoke/all` で再現せず全PASSを確認した。
