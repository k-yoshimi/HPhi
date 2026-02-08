Date: 2026-02-06
Time: 22:28:50 JST

# Refactoring Session 018 (REF-012 continuation)

## 実施内容
- `sz` の unit カバレッジを general spin ケースへ拡張。
- 追加ファイル:
  - `test/unit/sz_generalspin_characterization.sh`
- 登録更新:
  - `test/unit/CMakeLists.txt` に `unit_sz_generalspin_characterization` を追加。

## テスト仕様
- 入力: `Spin`, `L=4`, `2S=3`, `2Sz=0`, `Lanczos`, `chain`。
- 検証:
  - `output/CHECK_Sdim.dat` に `3 64` が存在。
  - `output/CHECK_Memory.dat` に `idim_max=44` が存在。
  - `output/Err_sz.dat` が生成されない。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
