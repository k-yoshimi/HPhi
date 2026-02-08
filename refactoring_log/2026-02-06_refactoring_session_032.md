Date: 2026-02-06
Time: 23:13:26 JST

# Refactoring Session 032 (REF-012 continuation)

## 実施内容
- TE diagonal 系のcharacterizationを `unit` に追加。
  - 追加ファイル: `test/unit/te_interall_diagonal_characterization.sh`
  - 追加テスト名: `unit_te_interall_diagonal_characterization`
- `test/testTECalc.py -t Diagonal` 実行後、`output/Flct.dat` の代表2点を参照値と比較する構成にした。

## 修正対応
- 新規unitの初回失敗原因は `testTECalc.py` への相対パス不整合だったため、以下へ修正。
  - `TE_TOOL="../../../../test/testTECalc.py"`

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (10/10)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (70/70)
