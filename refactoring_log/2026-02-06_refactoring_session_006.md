Date: 2026-02-06
Time: 21:43:42 JST

# Refactoring Session 006 (REF-010)

## 実施内容
- `test/unit/sz_characterization.sh` を新規追加。
- `test/unit/CMakeLists.txt` に `unit_sz_characterization` を追加（`unit` ラベル）。
- 小規模ケースで `sz` 系の特性を固定:
  - Spin (`L=4`) で `output/CHECK_Sdim.dat` の先頭行が `sdim=4 =2^2`。
  - Hubbard (`L=4`, `nelec=4`) で `output/CHECK_Sdim.dat` の先頭行が `sdim=16 =2^4`。
  - 併せて `output/CHECK_Memory.dat` の `idim_max` が既知値 (`6`, `36`) であることを確認。

## 途中障害と対処
- `ctest -L smoke` と `ctest` を同時実行してテスト作業ディレクトリが競合し、一時的に `te_ac_hubbard_square` が失敗。
- 直列実行に切り替えて再確認し、全テスト成功を確認。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (4/4)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (64/64)
