Date: 2026-02-06
Time: 20:46:40 JST

# Refactoring Work Log

## Entry 001
- Request: keep writing progress logs under `refactoring_log/` as Markdown files.
- Rule applied: each Markdown log starts with `Date` and `Time` at the top.
- Action taken:
  - Confirmed `refactoring_log/` exists.
  - Confirmed `refactoring_log/plan_refactoring.md` is present.
  - Created this ongoing log file for future updates.

## Entry 002
- Date: 2026-02-06
- Time: 22:16:05 JST
- REF-012時点の問題点を記録:
  - `sz()` 冒頭に未使用ローカル変数が多数残っており、警告が多い。
  - `Kondo` モデルで `CalcHS(read_hacker)` の不正値時に明示エラーにならない経路がある。
  - `Spin` の `hacker==-1` 経路は暗黙的な `icnt` 依存が残る。
  - `unit` の characterization が `Kondo/tJ/general spin` や `CalcHS` 異常値分岐を直接カバーしていない。
  - `sz.c` / `diagonalcalc.c` は依然として巨大で、段階分割は継続が必要。

## Entry 003
- Date: 2026-02-06
- Time: 22:21:29 JST
- REF-012 継続対応:
  - `src/sz.c` の `Spin` + `CalcHS=-1` 経路で `i_max` が `1` になって `Err_sz` で停止する不具合を再現確認。
  - `ComputeSzCountForSpin` の `hacker==-1` 分岐で、legacy 経路の計算後に `icnt = X->Check.idim_max;` を明示設定して整合を回復。
  - 新規 unit test `test/unit/sz_spin_calchs_minus1.sh` を追加し、`CalcHS=-1` で `Spin` 計算が完走することを固定化。
  - `test/unit/CMakeLists.txt` に `unit_sz_spin_calchs_minus1` を追加。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (7/7)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (67/67)

## Entry 004
- Date: 2026-02-06
- Time: 22:23:09 JST
- REF-012 継続対応（追加）:
  - `src/sz.c`
    - `calculate_jb_Spin_Old` の `jb` 未初期化を修正（`jb = 0` を明示）。
    - `calculate_jb_Spin_m1` の未使用ローカル変数を整理。
  - `test/unit/sz_spin_calchs_zero.sh` を追加し、`Spin` + `CalcHS=0` 経路を unit で固定化。
  - `test/unit/CMakeLists.txt` に `unit_sz_spin_calchs_zero` を追加。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (8/8)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (68/68)
- 備考:
  - `src/sz.c` の警告は 42 -> 35 へ減少。残警告は次段で継続対応。

## Entry 005
- Date: 2026-02-06
- Time: 22:26:47 JST
- REF-012 継続対応（warning cleanup in `sz.c`）:
  - `omp_sz_KondoNConserved` / `omp_sz_Kondo` / `omp_sz_Kondo_hacker` / `omp_sz_KondoGC` の自己代入 (`icheck_loc = icheck_loc`) を削除し、同値条件式へ整理。
  - `calculate_jb_Kondo` / `calculate_jb_KondoNConserved` / `calculate_jb_HubbardNCoserved` / `calculate_jb_tJGC` の未使用ローカル変数を削減。
  - `calculate_jb_tJGC` で未使用だった `num_up/num_down` の更新処理を除去（`check_doublon` 判定のみ維持）。
- 結果:
  - `src/sz.c` の compile warning を解消（前回 35 件 -> 0 件）。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (8/8)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (68/68)

## Entry 006
- Date: 2026-02-06
- Time: 22:28:50 JST
- REF-012 継続対応（unit coverage 拡張）:
  - `test/unit/sz_generalspin_characterization.sh` を追加。
    - `Spin` + `2S=3`（general spin）ケースを `-s` で実行。
    - `CHECK_Sdim.dat` の分割次元行（`3 64`）と `CHECK_Memory.dat` の `idim_max=44` を検証。
    - `output/Err_sz.dat` 未生成を確認。
  - `test/unit/CMakeLists.txt` に `unit_sz_generalspin_characterization` を登録。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 007
- Date: 2026-02-06
- Time: 22:30:33 JST
- REF-012 継続対応（`diagonalcalcForTE` の分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyTETransferDiagonalStep`
    - `ApplyTEInterAllDiagonalStep`
    - `ApplyTEChemiStep`
  - `diagonalcalcForTE` 本体は helper 呼び出しへ置換。
  - 既存の分岐優先順位（`NTETransferDiagonal > 0` を優先し、そうでない場合のみ `NTEInterAllDiagonal` 経路）を維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 008
- Date: 2026-02-06
- Time: 22:37:17 JST
- REF-012 継続対応（`SetDiagonalCoulombIntra` の分割）:
  - `src/diagonalcalc.c` に内部 helper を追加。
    - `AddDiagonalConstantTerm`
    - `AddDiagonalMaskedBySequentialIndex`
    - `AddDiagonalMaskedByRestrictedBasis`
  - `SetDiagonalCoulombIntra` 本体の OpenMP ループを上記 helper 呼び出しへ置換。
  - マスク計算（`Tpow[2*isite1-2] + Tpow[2*isite1-1]`）とモデル分岐の挙動は維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 009
- Date: 2026-02-06
- Time: 22:38:47 JST
- REF-012 継続対応（`SetDiagonalChemi` の分割）:
  - `src/diagonalcalc.c` に `ApplyChemiInterProcess` を追加。
  - `SetDiagonalChemi` の `isite1 > Nsite` 分岐を helper 呼び出しへ置換。
  - `isite1 <= Nsite` 側の既存ロジックは未変更。
- 挙動維持:
  - モデル分岐（Hubbard/Kondo/tJ, Spin/SpinGC）と `BitCheckGeneral` 経路をそのまま移設。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 010
- Date: 2026-02-06
- Time: 22:40:16 JST
- REF-012 継続対応（`SetDiagonalCoulombInter` の先頭分岐を分割）:
  - `src/diagonalcalc.c` に `ApplyCoulombInterBothInterProcess` を追加。
  - `SetDiagonalCoulombInter` の `isite1 > Nsite` 分岐を helper 呼び出しに置換。
  - `isite2 > Nsite` 分岐および両サイト intra-process 分岐は未変更。
- 挙動維持:
  - model別処理（Hubbard/Kondo/tJ と Spin/SpinGC）および `num1*num2*dtmp_V` の更新式をそのまま移設。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 011
- Date: 2026-02-06
- Time: 22:48:34 JST
- REF-012 継続対応（`SetDiagonalCoulombInter` の残り分岐を分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyCoulombInterOneInterProcess`（`isite1 <= Nsite < isite2`）
    - `ApplyCoulombInterIntraProcess`（`isite1, isite2 <= Nsite`）
  - `SetDiagonalCoulombInter` は3分岐すべて helper 呼び出し構造へ整理。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 012
- Date: 2026-02-06
- Time: 22:48:34 JST
- REF-012 継続対応（`SetDiagonalHund` 先頭分岐の分割）:
  - `src/diagonalcalc.c` に `ApplyHundBothInterProcess` を追加。
  - `SetDiagonalHund` の `isite1 > Nsite`（両サイト inter-process）分岐を helper 化。
  - `isite2 > Nsite` 分岐および両サイト intra-process 分岐は未変更。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 013
- Date: 2026-02-06
- Time: 22:51:46 JST
- REF-012 継続対応（`SetDiagonalHund` の残り分岐を分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyHundOneInterProcess`（`isite1 <= Nsite < isite2`）
    - `ApplyHundIntraProcess`（`isite1, isite2 <= Nsite`）
  - `SetDiagonalHund` は3分岐 dispatcher へ整理。
    - `ApplyHundBothInterProcess`
    - `ApplyHundOneInterProcess`
    - `ApplyHundIntraProcess`
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 014
- Date: 2026-02-06
- Time: 22:53:32 JST
- REF-012 継続対応（`SetDiagonalInterAll` の先頭分岐を分割）:
  - `src/diagonalcalc.c` に `ApplyInterAllBothInterProcess` を追加。
  - `SetDiagonalInterAll` の `isite1 > Nsite`（両サイト inter-process）分岐を helper 化。
  - `isite2 > Nsite` 分岐および両サイト intra-process 分岐は未変更。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 015
- Date: 2026-02-06
- Time: 22:57:16 JST
- REF-012 継続対応（`SetDiagonalInterAll` の残り分岐を分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyInterAllOneInterProcess`（`isite1 <= Nsite < isite2`）
    - `ApplyInterAllIntraProcess`（`isite1, isite2 <= Nsite`）
  - `SetDiagonalInterAll` は3分岐 dispatcher へ整理。
    - `ApplyInterAllBothInterProcess`
    - `ApplyInterAllOneInterProcess`
    - `ApplyInterAllIntraProcess`
  - 追加分割後に `ApplyInterAllIntraProcess` の未使用ローカル（`is_up`, `ibit`）を削除。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 016
- Date: 2026-02-06
- Time: 23:01:16 JST
- REF-012 継続対応（`SetDiagonalTEInterAll` の段階分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyTEInterAllBothInterProcess`（`isite1, isite2 > Nsite`）
    - `ApplyTEInterAllOneInterProcess`（`isite1 <= Nsite < isite2`）
    - `ApplyTEInterAllIntraProcess`（`isite1, isite2 <= Nsite`）
  - `SetDiagonalTEInterAll` 本体は swap + dispatcher の構造へ整理。
- 挙動維持:
  - model別分岐、`child_Spin*` / `BitCheckGeneral` 条件、`dam_pr` 集計と `X->Large.prdct` 更新の位置を維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 017
- Date: 2026-02-06
- Time: 23:05:05 JST
- REF-012 継続対応（`SetDiagonalTEChemi` の分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyTEChemiInterProcess`（`isite1 > Nsite`）
    - `ApplyTEChemiIntraProcess`（`isite1 <= Nsite`）
  - `SetDiagonalTEChemi` 本体を dispatcher 化。
- 挙動維持:
  - model分岐、`BitCheckGeneral`/`child_Spin*` 条件、`dam_pr` 集計と `X->Large.prdct` 更新を維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 018
- Date: 2026-02-06
- Time: 23:08:46 JST
- REF-012 継続対応（`SetDiagonalTETransfer` の分割）:
  - `src/diagonalcalc.c` に以下 helper を追加。
    - `ApplyTETransferInterProcess`（`isite1 > Nsite`）
    - `ApplyTETransferIntraProcess`（`isite1 <= Nsite`）
  - `SetDiagonalTETransfer` 本体を dispatcher 化。
- 挙動維持:
  - model分岐、`BitCheckGeneral`/`child_Spin*` 条件、`dam_pr` 集計と `X->Large.prdct` 更新を維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 019
- Date: 2026-02-06
- Time: 23:11:26 JST
- REF-012 継続対応（`SetDiagonalChemi` の残り分岐を分割）:
  - `src/diagonalcalc.c` に `ApplyChemiIntraProcess` を追加。
  - `SetDiagonalChemi` は以下2分岐の dispatcher へ整理。
    - `isite1 > Nsite`: `ApplyChemiInterProcess`
    - `isite1 <= Nsite`: `ApplyChemiIntraProcess`
- 挙動維持:
  - model分岐、`BitCheckGeneral` 条件、`list_Diagonal` 更新式を変更せず移設。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (9/9)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (69/69)

## Entry 020
- Date: 2026-02-06
- Time: 23:13:26 JST
- REF-012 継続対応（TE diagonal 系のunit characterization追加）:
  - 新規: `test/unit/te_interall_diagonal_characterization.sh`
    - `test/testTECalc.py -t Diagonal` を実行し、`output/Flct.dat` の先頭/末尾代表点を参照値と比較。
  - `test/unit/CMakeLists.txt` に `unit_te_interall_diagonal_characterization` を追加。
  - 初回実行で `testTECalc.py` の相対パス不整合を修正（`../../../../test/testTECalc.py`）。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (10/10)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (70/70)

## Entry 021
- Date: 2026-02-06
- Time: 23:27:08 JST
- REF-009 着手（`readdef.c` の switch 可読性改善: 第1段）:
  - `src/readdef.c` に `ReadDefFileNInt` 用の dispatcher を追加。
    - `ReadNIntContext`, `ReadNIntDispatchEntry`, `DispatchReadNIntKeyword`
  - `ReadDefFileNInt` の巨大 `switch(iKWidx)` を、`keyword -> handler` 方式へ置換。
  - 必須キーワード判定を `IsRequiredNameListKeyword` に抽出。
  - `ModPara` 読み込み処理を `HandleNIntModPara` へ分離し、`ReadDefFileNInt` 本体から切り離し。
- 挙動維持:
  - 既存のキーワード対応範囲（`CalcMod`〜`PairExcitation`）は同一。
  - `NPairHopping` の2倍化、TE系 max カウント算出、Boost読み込み処理はそのまま移設。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (10/10)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (70/70)

## Entry 022
- Date: 2026-02-06
- Time: 23:31:56 JST
- REF-009 継続（`ReadDefFileIdxPara` の switch 可読性改善: 第2段）:
  - `src/readdef.c` に以下 parser helper を追加し、`ReadDefFileIdxPara` のケース本体を移設。
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
  - `ReadDefFileIdxPara` の `KWLocSpin`〜`KWPairLift`、`KWOneBodyG`、`KWTwoBodyG` は helper 呼び出しに置換。
- 挙動維持:
  - 既存のチェック（site/pair/quad、Hermite、model別制約）とエラー経路を維持。
  - `PairHop` の mirror 登録、`Ising` の Hund/CoulombInter 反映も維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (10/10)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (70/70)

## Entry 023
- Date: 2026-02-06
- Time: 23:41:29 JST
- REF-009 継続（`ReadDefFileIdxPara` の switch 可読性改善: 第3段）:
  - `src/readdef.c` に以下 parser helper を追加。
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
  - `ReadDefFileIdxPara` の `KWInterAll` 以降の大型ケース本体を helper 呼び出しに置換。
  - `ReadDefFileIdxPara` の未使用ローカル変数を整理。
- 注意点:
  - 一時的に `ctest -L smoke` と `ctest` 全体を同時実行して競合し、`te_ac_hubbard_square` が失敗。
  - 逐次実行へ修正後、同一コミット内容で全通過を確認。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (10/10)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (70/70)

## Entry 024
- Date: 2026-02-06
- Time: 23:46:30 JST
- REF-009 継続（`ReadDefFileIdxPara` の dispatcher 化: 第4段）:
  - `src/readdef.c` に `ReadDefFileIdxPara` 用 dispatcher を追加。
    - `ReadIdxContext`
    - `ReadIdxDispatchEntry`
    - `DispatchReadIdxKeyword`
    - `IsGeneralSpinForbiddenKeyword`
  - `ParseIdx*` helper 呼び出しを `HandleIdx*` handler にまとめ、`keyword -> handler` のテーブル駆動へ置換。
  - `ReadDefFileIdxPara` 本体から巨大 `switch` 2つを削除し、`dispatch + post-check` 構成へ簡素化。
- 挙動維持:
  - 既存の parser helper 群は再利用し、パース内容・エラー経路・general-spin 制約判定を維持。
  - `KWInvTemp` など非対応キーワードは従来どおり no-op 扱い（default相当）を維持。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (10/10)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (70/70)

## Entry 025
- Date: 2026-02-06
- Time: 23:53:54 JST
- REF-009 継続（`readdef` parser 分離と readdef characterization 拡張）:
  - `src/readdef_idx_parser.c` を新規追加し、`ReadDefFileIdxPara` 用 parser/disptacher 群を移設。
    - `ParseIdx*` 系 helper 一式
    - `HandleIdx*` / `DispatchReadIdxKeyword`
    - `IsGeneralSpinForbiddenKeyword`
    - `ParseReadDefIdxKeyword`（`readdef.c` からの公開呼び出し口）
  - `src/include/readdef_idx_parser.h` を新規追加。
  - `src/readdef.c` は `ReadDefFileIdxPara` から parser 実装詳細を削除し、`ParseReadDefIdxKeyword` 呼び出しのみへ簡素化。
  - `src/CMakeLists.txt` に `readdef_idx_parser.c` を追加。
- REF-008/009 補強（readdef parser characterization 追加）:
  - 新規: `test/unit/readdef_pair_excitation_count_mismatch.sh`
    - `PairExcitation` の件数不整合（header count と実データ不一致）を作り、エラー検出を確認。
  - 新規: `test/unit/readdef_teonebody_count_mismatch.sh`
    - `TEOneBody` の timestep 件数不整合を作り、エラー検出を確認。
  - `test/unit/CMakeLists.txt` に上記2テストを追加（label: `unit`）。
- 検証結果:
  - `ctest --output-on-failure -L unit -j4`: PASS (12/12)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (72/72)

## Entry 026
- Date: 2026-02-07
- Time: 00:02:40 JST
- 直近タスク（1 -> 5）の完了対応:
  - `test/unit/CMakeLists.txt` に以下2テストを登録。
    - `unit_sz_kondogc_characterization`
    - `unit_diagonalcalc_kondo_characterization`
  - REF-013: `doc/test_operations.md` を追加済みで、運用手順を明文化。
  - REF-009: `src/readdef.c` の `ReadDefFileNInt` 末尾処理を helper 分割して可読性改善。
  - REF-012: `src/sz.c`/`src/diagonalcalc.c` で重複処理を helper 化し、Kondo系 characterization を追加。
  - REF-006: `src/bitcalc.c` で `GetNsiteMultiplierByModel` を導入し、`GetSplitBitByModel` の責務を整理。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (14/14)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (74/74)

## Entry 027
- Date: 2026-02-07
- Time: 00:07:47 JST
- 残タスク対応（`readdef.c` error path改善 + test追加）:
  - `src/readdef.c`
    - `ReadDefFileNInt` の処理を helper へ追加分割:
      - `CheckRequiredNameListFiles`
      - `ReadNIntKeywordFile`
      - `PostprocessReadDefNInt`
      - `GetReadNIntValidationContext`
    - これにより「必須ファイル検証」「1ファイル読込とdispatch」「後処理と検証」を分離。
    - `ValidateReadNIntPositiveValues` のエラー出力名は `ModPara` を優先して使うよう統一。
  - `test/unit/readdef_missing_ncond_error.sh` を追加。
    - `modpara.def` から `Ncond/Nup/Ndown/2Sz` を除去し、`NCond is not defined` の失敗経路を固定化。
  - `test/unit/CMakeLists.txt` に `unit_readdef_missing_ncond_error` を登録。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (15/15)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (75/75)

## Entry 028
- Date: 2026-02-07
- Time: 00:14:44 JST
- 残タスク対応（`readdef` parser API境界の追加分離）:
  - 新規: `src/readdef_nint_parser.c`
    - `ReadDefFileNInt` 側 keyword parser/dispatcher (`HandleNInt*`) を `readdef.c` から移設。
    - 公開 API: `ParseReadDefNIntKeyword`
    - 必須keyword判定 API: `IsRequiredNameListKeyword`
  - 新規: `src/include/readdef_nint_parser.h`
  - `src/readdef.c`
    - `HandleNInt*` 群と `DispatchReadNIntKeyword`、`ReadNIntContext` を削除。
    - `ReadNIntKeywordFile` で `ParseReadDefNIntKeyword` を呼ぶ構成へ変更。
  - `src/CMakeLists.txt`
    - `readdef_nint_parser.c` を `SOURCES` に追加。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (15/15)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (75/75)

## Entry 029
- Date: 2026-02-07
- Time: 00:20:47 JST
- 残タスク対応（`sz`/`diagonalcalc` 継続）:
  - `src/diagonalcalc.c`
    - `ApplyTETransferIntraProcess` を model別 helper へ分割。
      - `GetTETransferMask`
      - `AccumulateTETransferHubbardGC`
      - `AccumulateTETransferRestrictedBasis`
      - `AccumulateTETransferSpinGC`
      - `AccumulateTETransferSpin`
    - model分岐の見通しを改善し、各計算経路を関数単位で追跡可能に整理。
  - `test/unit/sz_spingc_characterization.sh` を追加。
    - `SpinGC` の `sdim/idim_max` を characterization で固定化。
  - `test/unit/CMakeLists.txt` に `unit_sz_spingc_characterization` を登録。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)

## Entry 030
- Date: 2026-02-07
- Time: 00:24:13 JST
- 残タスク対応（`sz.c` の `calculate_jb_*` 継続分割）:
  - `src/sz.c`
    - 新規 helper:
      - `GetTJHalfBitSiteCounts`
      - `CountTJHalfBitOccupations`
    - `calculate_jb_tJ` / `calculate_jb_tJNConserved` / `calculate_jb_tJGC` の重複処理を helper 経由に整理。
      - doublon 判定
      - half-bit 側の up/down 数カウント
      - `all_up/all_down` 算出
    - 挙動変更は入れず、分岐と計算式は維持。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)

## Entry 031
- Date: 2026-02-07
- Time: 00:32:18 JST
- 残タスク対応（`sz.c` の `calculate_jb_*` 継続分割: Kondo系）:
  - `src/sz.c`
    - 追加 helper:
      - `GetKondoSiteOccupations`
      - `CountKondoRightHalfOccupations`
      - `GetKondoHalfBitCombinationWindow`
      - `ComputeKondoFixedParticleContribution`
      - `HasValidKondoGCLocalizedConfiguration`
    - 対象関数:
      - `calculate_jb_Kondo`
      - `calculate_jb_KondoNConserved`
      - `calculate_jb_KondoGC`
    - 右半分bitのoccupation抽出、局在スピン制約判定、組合せ寄与計算を helper に集約。
    - `count_localized_spins` を Kondo/KondoNConserved 側でも再利用し、局在サイト数計算重複を削減。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 備考:
  - `ctest -L smoke` 実行中に `te_ac_hubbard_square` が1回だけ失敗し、依存の3件が `Not Run` になった。
  - `ctest -R '^te_ac_hubbard_square$'` と `ctest -L smoke -j4`、`ctest -j4` の再実行では再現せず、すべてPASSを確認。

## Entry 032
- Date: 2026-02-07
- Time: 00:35:09 JST
- 残タスク対応（`diagonalcalc.c` inter process 重複整理）:
  - `src/diagonalcalc.c`
    - 新規 helper:
      - `ApplyTEOneBodyInterProcess`
    - `ApplyTEChemiInterProcess` と `ApplyTETransferInterProcess` から、
      model別 occupation 判定・`tmp_v0` 更新・`dam_pr` 累積の共通実装を helper に統合。
    - TEChemi/TETransfer の inter process は wrapper から共通 helper 呼び出しへ変更。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 備考:
  - 初回の `smoke/all` 実行で `te_ac_hubbard_square` が単発失敗し、依存3件が `Not Run` になった。
  - `ctest -R '^te_ac_hubbard_square$'`、`ctest -L smoke -j4`、`ctest -j4` 再実行で再現せず、最終的に全PASSを確認。

## Entry 033
- Date: 2026-02-07
- Time: 00:41:14 JST
- 残タスク対応（`diagonalcalc.c` intra 重複統合 + characterization拡張）:
  - `src/diagonalcalc.c`
    - `ApplyTEChemiIntraProcess` を整理。
      - `Spin` + general-spin は専用 helper `AccumulateTEChemiSpinGeneral` で従来挙動を維持。
      - それ以外の model は `ApplyTETransferIntraProcess` を再利用して重複分岐を削減。
    - `ApplyTETransferIntraProcess` の forward declaration を追加し、intra one-body 共通経路を明確化。
  - `test/unit/sz_kondo_nconserved_characterization.sh` を追加。
    - `Kondo` で `2Sz` 未指定ケース（NConserved）を実行し、
      `sdim=256` / `idim_max=1120` / `Err_sz.dat` 非生成を固定化。
  - `test/unit/CMakeLists.txt`
    - `unit_sz_kondo_nconserved_characterization` を登録（label: `unit`）。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (17/17)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (77/77)
- 備考:
  - `smoke` と `all` を同時に走らせると `te_ac_hubbard_square` が干渉で失敗し得るため、
    最終確認は `te_ac` 単体 -> `smoke` -> `all` の順で逐次実行して PASS を確認。

## Entry 034
- Date: 2026-02-07
- Time: 00:45:47 JST
- 残タスク対応（characterization 拡張: SpinGC）:
  - 新規: `test/unit/diagonalcalc_spingc_characterization.sh`
    - `SpinGC` + `FullDiag`（L=3 chain）を実行。
    - `output/CHECK_Memory.dat` の `idim_max=8` を検証。
    - `output/zvo_phys.dat` の先頭3固有値（`-0.750000`）を固定化。
    - 行数（header含め9行）を検証。
  - `test/unit/CMakeLists.txt`
    - `unit_diagonalcalc_spingc_characterization` を登録（label: `unit`）。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (18/18)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (78/78)
- 備考:
  - tJ characterization を追加しようとしたが、standard mode では `model="tJ"` が未対応のため、
    追加時は expert mode 入力ファイル群の fixture 化が必要。

## Entry 035
- Date: 2026-02-07
- Time: 00:49:53 JST
- 残タスク対応（tJ expert mode fixture の unit test 化）:
  - 新規: `test/unit/sz_tj_expert_characterization.sh`
    - expert mode `.def` fixture をテスト内で生成。
      - `calcmod.def`: `CalcModel=9`（tJ）
      - `modpara.def`: `Nsite=4`, `Ncond=3`（`2Sz` あり/なしの2ケース）
      - `locspn.def`: 全サイト itinerant
      - `trans.def`: 最小 transfer 4本
    - ケース1（`2Sz` あり）で tJ を検証:
      - `sdim=16`, `idim_max=12`
    - ケース2（`2Sz` なし）で tJNConserved を検証:
      - `sdim=16`, `idim_max=32`
  - `test/unit/CMakeLists.txt`
    - `unit_sz_tj_expert_characterization` を追加（label: `unit`）。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (19/19)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (79/79)

## Entry 036
- Date: 2026-02-07
- Time: 00:55:41 JST
- 残タスク対応（tJGC expert mode fixture + dimension mismatch 修正）:
  - `src/check.c`
    - `case tJGC` の `idim_max` 計算ループを `Ne` 依存から `Nsite` 依存へ修正。
    - `Ne` が 0 の入力でも `tJGC` の期待次元（`3^Nsite`）を算出するように変更。
  - 新規: `test/unit/sz_tjgc_expert_characterization.sh`
    - expert mode `.def` fixture を生成し、`CalcModel=10`（tJGC）を直接検証。
    - `Nsite=4` で `sdim=16`, `idim_max=81`, `Err_sz.dat` 非生成を固定化。
  - `test/unit/CMakeLists.txt`
    - `unit_sz_tjgc_expert_characterization` を追加（label: `unit`）。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)

## Entry 037
- Date: 2026-02-07
- Time: 00:59:50 JST
- 残タスク対応（`readdef.c` switch-case 可読性改善 + test運用明文化）:
  - `src/readdef.c`
    - `ApplyReadNIntModelRules` を小関数に分割して責務を明確化。
      - canonical/GC 判定 helper
      - `Ncond` + `2Sz` あり/なしの処理 helper
      - canonical 共通後処理 helper
    - `CheckLocSpin` のモデル別分岐を helper 化し、長い `switch` を分類関数 + 検証関数へ整理。
  - `doc/test_operations.md`
    - `te_ac_hubbard_square` の並列実行干渉に関する `Known Pitfall` を追記。
    - 失敗時の isolation 再実行コマンドを明記。
- 検証結果:
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)

## Entry 038
- Date: 2026-02-07
- Time: 01:05:58 JST
- 残タスク対応（指示順: 2 -> 1）:
  - `2` CI 側の干渉回避を先行実施。
    - `.github/workflows/main.yml`
      - job を `ctest_unit_smoke`（push/pr）と `ctest_full_schedule`（schedule）に分離。
      - workflow-level `concurrency` を追加し、同一 ref の実行を排他化。
  - `1` `readdef.c` の大型 switch を続けて整理。
    - `GetDiagonalInterAll` の off-diagonal model 分岐を helper 抽出で簡素化。
    - `ArrangeInterAllOffDiagonal` の model 分岐を helper 抽出で簡素化。
    - 追加 helper:
      - `IsInterAllFermionFamilyModel`
      - `IsInterAllSpinFamilyModel`
      - `IsInterAllStrictSzModel`
      - `IsArrangeInterAllModel`
      - `CopyInterAllRow8`
      - `SetInterAllExchangeRow`
      - `BuildInterAllOffDiagonalTerm`
      - `NormalizeInterAllOffDiagonalRow`
- 検証結果:
  - `ruby -e 'require "yaml"; YAML.load_file(".github/workflows/main.yml")'`: PASS
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)

## Entry 039
- Date: 2026-02-07
- Time: 01:10:52 JST
- 残タスク対応（1,2 完了版）:
  - `1` `readdef.c` 重複整理の追加統合。
    - `GetDiagonalInterAll` と `GetDiagonalInterAll_simple` で重複していた diagonal 判定/反映処理を helper 化。
      - `InterAllTerm`
      - `LoadInterAllTermFromRow`
      - `AppendDiagonalInterAllTerm`
    - `GetDiagonalInterAll_simple` の off-diagonal コピーは `CopyInterAllRow8` を再利用。
  - `2` CI の重複 step 共通化。
    - 新規 composite action: `.github/actions/bootstrap-build/action.yml`
    - `.github/workflows/main.yml` の両 job で、依存導入〜build の重複を action 呼び出しへ置換。
- 検証結果:
  - `ruby -e 'require "yaml"; YAML.load_file(".github/workflows/main.yml"); YAML.load_file(".github/actions/bootstrap-build/action.yml")'`: PASS
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)

## Entry 040
- Date: 2026-02-07
- Time: 01:14:23 JST
- 残タスク対応（1,2 の追加改善）:
  - `1` `GetDiagonalInterAll_simple` の役割整理を実装統合で前進。
    - `SplitDiagonalAndOffDiagonalInterAll` を追加し、`GetDiagonalInterAll` と `GetDiagonalInterAll_simple` を core 共有化。
    - diagonal 分岐処理を `InterAllTerm` / `LoadInterAllTermFromRow` / `AppendDiagonalInterAllTerm` に集約。
    - APIは維持しつつ、`apply_model_filter` フラグで挙動差（simple/normal）を保持。
  - `2` CI matrix 最適化を追加実施。
    - push/pr を `ctest_unit` と `ctest_smoke` の2 jobへ分離。
    - `unit` は ubuntu 単一構成に固定し、重複実行を削減。
    - `smoke` は matrix 維持で platform/omp coverage を継続。
- 検証結果:
  - `ruby -e 'require "yaml"; YAML.load_file(".github/workflows/main.yml"); YAML.load_file(".github/actions/bootstrap-build/action.yml")'`: PASS
  - `cmake --build build -j4`: PASS
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)
