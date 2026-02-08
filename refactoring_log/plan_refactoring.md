Date: 2026-02-06
Time: 20:43:12 JST

# HPhi Refactoring and Unit Test Plan

## 目的
- 既存の数値結果と挙動を維持したまま、コードの保守性を上げる。
- 既存の回帰テスト（E2E）を土台に、unit testを段階的に導入する。
- リファクタリングのリスクを下げるため、低リスク領域から高リスク領域へ順次進める。

## 現状の前提
- 既存テストは`test/`配下のシェルスクリプトを`ctest`で実行する回帰テスト中心。
- CI（`.github/workflows/main.yml`）ですでに`ctest`が実行されている。
- `global.h`依存が強く、巨大ファイル（例: `readdef.c`, `sz.c`, `diagonalcalc.c`）の一括変更は高リスク。

## 開発方針
- 「既存回帰テストを守る」ことを最優先にする。
- 「テスト追加 -> 小さなリファクタ」の順で反復する。
- 1PR 1責務で変更範囲を限定する。
- 挙動変更を伴わない整理（関数分割、命名整理、依存整理）を先行する。

## 実施順序
1. ベースライン固定  
   現行`ctest`結果を基準値として固定し、破壊禁止の挙動を明文化する。
2. テスト階層の整理  
   `E2E(既存)`と`Unit(新規)`を明確に分離し、目的と実行時間を区別する。
3. Unit test基盤の追加  
   `test/unit/`を作成し、CMakeで高速に実行できるunit testターゲットを用意する。
4. 低リスク領域へのunit test導入  
   `bitcalc.c`、`common/setmemory.c`など副作用が少ない関数群から着手する。
5. 小分けリファクタの反復  
   unit testで保護しながら、関数分割・責務分離を少しずつ進める。
6. 中リスク領域へ拡張  
   `readdef.c`を「パース」「検証」「設定反映」に分離し、関数単位テストを追加する。
7. 高リスク領域へ拡張  
   `sz.c`、`diagonalcalc.c`はcharacterization testを先に作ってから分割する。
8. CI最適化  
   PRでは`unit + smoke E2E`、定期実行ではフル`ctest`を維持する。

## 最初の3PR（推奨）
1. テスト実行整理  
   `test/CMakeLists.txt`へラベル（例: `smoke`, `slow`）を追加し、実行導線を整理する。
2. unit test土台導入  
   `test/unit/CMakeLists.txt`と最小テストランナーを追加する。
3. 初回unit test追加  
   `bitcalc`系関数のunit test追加と、挙動不変の軽微リファクタを実施する。

## 完了条件
- unit testがCIで常時実行される。
- 主要モジュールに対して最小限のunit testカバレッジがある。
- 既存E2Eテストで数値回帰が発生していない。
- 大規模ファイルの責務分割方針がドキュメント化され、継続可能な状態になっている。

## 実施チケット一覧（優先度・工数）

| ID | タイトル | 優先度 | 工数(人日) | 依存 | 成果物 |
|---|---|---|---:|---|---|
| REF-001 | 現行`ctest`ベースライン固定と記録 | P0 | 0.5 | なし | 実行コマンド・結果ログ・失敗時方針 |
| REF-002 | 既存E2Eテストへのラベル付け（`smoke`/`slow`/`mpi`） | P0 | 1.0 | REF-001 | `test/CMakeLists.txt`更新 |
| REF-003 | CIを`unit + smoke`（PR）/`full`（定期）に分離 | P0 | 1.0 | REF-002 | `.github/workflows/main.yml`更新 |
| REF-004 | `test/unit/`とCMake連携の最小unit test基盤作成 | P0 | 1.5 | REF-002 | `test/unit/CMakeLists.txt`、最小テスト実行 |
| REF-005 | `bitcalc`のunit test追加（境界値・正常系・異常系） | P0 | 2.0 | REF-004 | `test/unit/test_bitcalc*.c` |
| REF-006 | `bitcalc`軽微リファクタ（挙動不変、責務明確化） | P1 | 1.5 | REF-005 | `src/bitcalc.c`分割・整理 |
| REF-007 | `common/setmemory`系のunit test追加 | P1 | 1.5 | REF-004 | `test/unit/test_setmemory*.c` |
| REF-008 | `readdef`のcharacterization test追加（代表入力群） | P1 | 2.0 | REF-004 | `test/unit`または`test/fixtures` |
| REF-009 | `readdef`をパース/検証/反映に段階分割 | P1 | 3.0 | REF-008 | `src/readdef.c`分割、関連ヘッダ整理 |
| REF-010 | `sz`のcharacterization test追加（小規模系） | P2 | 2.0 | REF-001 | 既知系の次元・量子数検証テスト |
| REF-011 | `diagonalcalc`のcharacterization test追加 | P2 | 2.0 | REF-001 | 固有値・収束条件の回帰テスト |
| REF-012 | `sz`/`diagonalcalc`の段階的分割（挙動不変） | P2 | 4.0 | REF-010, REF-011 | 関数分離・依存整理 |
| REF-013 | テスト運用ドキュメント整備（実行方法・追加規約） | P1 | 0.5 | REF-004 | `doc/`内ガイド追記 |

### 合計見積
- P0: 6.0人日
- P1: 8.5人日
- P2: 8.0人日
- 全体: 22.5人日

### 実行順（推奨）
1. REF-001 -> REF-002 -> REF-003 -> REF-004
2. REF-005 -> REF-006 -> REF-007
3. REF-008 -> REF-009
4. REF-010 -> REF-011 -> REF-012
5. REF-013

## 進捗更新（2026-02-07 00:02 JST）
- 直近の実施指示（1 -> 5）を完了。
  - 1: REF-013として`doc/test_operations.md`を追加。
  - 2: REF-009として`ReadDefFileNInt`末尾処理をhelperへ分割。
  - 3: REF-012として`sz.c`/`diagonalcalc.c`の重複処理を追加分割し、Kondo系characterization testを2件追加。
  - 4: REF-006として`src/bitcalc.c`の`GetSplitBitByModel`内部でモデル別倍率判定をhelper化（挙動不変）。
  - 5: plan/log同期を実施（本追記と`work_log`/session log更新）。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (14/14)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (74/74)
- 残タスク（継続改善）:
  - `readdef.c`の追加分割（error path含む）とparser API境界の明確化。
  - `sz.c`/`diagonalcalc.c`の長大関数分割の継続とcoverage拡充。

## 進捗更新（2026-02-07 00:07 JST）
- 残タスクのうち、`readdef.c`のerror path改善を追加実施。
  - `ReadDefFileNInt` の本体責務を追加分割。
    - `CheckRequiredNameListFiles`
    - `ReadNIntKeywordFile`
    - `PostprocessReadDefNInt`
    - `GetReadNIntValidationContext`
  - `ValidateReadNIntPositiveValues` のエラー表示コンテキストを `ModPara` 優先に固定（未設定時は `namelist` にフォールバック）。
- 追加unit test:
  - `unit_readdef_missing_ncond_error` を追加。
  - `Ncond/Nup/Ndown/2Sz` を欠落させた `modpara` で、`NCond is not defined` の失敗経路を固定化。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (15/15)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (75/75)
- 残タスク（継続改善）:
  - `readdef` parser API境界の更なる分離（`NInt` 側の別コンパイル単位化）。
  - `sz.c`/`diagonalcalc.c` の長大関数分割継続と characterization coverage 拡張。

## 進捗更新（2026-02-07 00:14 JST）
- 残タスクのうち、`readdef` parser API境界分離を追加実施。
  - 新規: `src/readdef_nint_parser.c`
  - 新規: `src/include/readdef_nint_parser.h`
  - `ReadDefFileNInt` の keyword dispatch (`HandleNInt*` 群) を `readdef.c` から切り出し。
  - `readdef.c` は `ParseReadDefNIntKeyword` 呼び出し中心に整理。
  - `src/CMakeLists.txt` に `readdef_nint_parser.c` を追加。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (15/15)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (75/75)
- 残タスク（継続改善）:
  - `sz.c`/`diagonalcalc.c` の長大関数分割継続。
  - `sz.c`/`diagonalcalc.c` の characterization coverage 拡張。

## 進捗更新（2026-02-07 00:20 JST）
- `diagonalcalc.c` の長大関数分割を継続。
  - `ApplyTETransferIntraProcess` を model別 helper へ分割。
    - `GetTETransferMask`
    - `AccumulateTETransferHubbardGC`
    - `AccumulateTETransferRestrictedBasis`
    - `AccumulateTETransferSpinGC`
    - `AccumulateTETransferSpin`
  - 挙動は同一のまま、分岐責務を helper 側に移動。
- characterization coverage を追加。
  - 新規 unit test: `unit_sz_spingc_characterization`
  - `SpinGC` の `CHECK_Sdim.dat` / `CHECK_Memory.dat` を固定化（`sdim=4`, `idim_max=16`）。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 残タスク（継続改善）:
  - `sz.c` の長大関数分割継続（`calculate_jb_*` 系）。
  - `diagonalcalc.c` の大型 helper（特に inter/intra 分岐の重複）整理継続。
  - `sz`/`diagonalcalc` の Kondo/tJ/SpinGC 系 characterization の追加拡充。

## 進捗更新（2026-02-07 00:24 JST）
- `sz.c` の `calculate_jb_*` 分割を継続（tJ系）。
  - 共通 helper を追加:
    - `GetTJHalfBitSiteCounts`
    - `CountTJHalfBitOccupations`
  - `calculate_jb_tJ` / `calculate_jb_tJNConserved` / `calculate_jb_tJGC` の重複していた
    - doublon 判定
    - half-bit 側の up/down 数カウント
    - `all_up/all_down` 計算
    を helper 経由に整理。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 残タスク（継続改善）:
  - `sz.c` の他 `calculate_jb_*` 系（Hubbard/Kondo系）の重複整理。
  - `diagonalcalc.c` の inter/intra 分岐の重複整理継続。
  - `sz`/`diagonalcalc` characterization 拡張（特に tJ/SpinGC の追加ケース）。

## 進捗更新（2026-02-07 00:27 JST）
- `sz.c` の `calculate_jb_*` 分割を継続（Hubbard系）。
  - 共通 helper を追加:
    - `GetHalfBitSiteCounts`
    - `CountHalfBitSpinOccupations`
    - `ComputeHubbardFixedParticleContribution`
    - `ComputeHubbardNConservedContribution`
  - 対象関数:
    - `calculate_jb_Hubbard`
    - `calculate_jb_Hubbard_Hacker`
    - `calculate_jb_HubbardNCoserved`
    - `calculate_jb_HubbardNCoserved_Hacker`
  - tJ系（前回追加分）も `GetHalfBitSiteCounts` に統一。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 残タスク（継続改善）:
  - `sz.c` の Kondo 系 `calculate_jb_*` の重複整理。
  - `diagonalcalc.c` の inter/intra 分岐の重複整理継続。
  - `sz`/`diagonalcalc` characterization の追加拡充（tJ/Kondo/SpinGC）。

## 進捗更新（2026-02-07 00:32 JST）
- `sz.c` の `calculate_jb_*` 分割を継続（Kondo系）。
  - 共通 helper を追加:
    - `GetKondoSiteOccupations`
    - `CountKondoRightHalfOccupations`
    - `GetKondoHalfBitCombinationWindow`
    - `ComputeKondoFixedParticleContribution`
    - `HasValidKondoGCLocalizedConfiguration`
  - 対象関数:
    - `calculate_jb_Kondo`
    - `calculate_jb_KondoNConserved`
    - `calculate_jb_KondoGC`
  - 右半分bitのoccupation抽出・局在スピン制約判定・組合せ寄与計算を helper 化し、重複分岐を整理。
- 検証結果（逐次実行）:
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 備考:
  - 途中実行の `smoke/all` で `te_ac_hubbard_square` が一度だけ失敗（連鎖で3件未実行）したが、単体再実行と再度の `smoke/all` では再現せずPASS。
- 残タスク（継続改善）:
  - `diagonalcalc.c` の inter/intra 分岐の重複整理継続。
  - `sz`/`diagonalcalc` characterization の追加拡充（tJ/Kondo/SpinGC）。

## 進捗更新（2026-02-07 00:35 JST）
- `diagonalcalc.c` の inter process 重複整理を実施。
  - 新規 helper:
    - `ApplyTEOneBodyInterProcess`
  - 置換対象:
    - `ApplyTEChemiInterProcess`
    - `ApplyTETransferInterProcess`
  - TEChemi/TETransfer の inter process 実装を共通 helper に統合し、model別 occupation 判定と `dam_pr` 更新の重複を解消。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (16/16)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (76/76)
- 備考:
  - `te_ac_hubbard_square` は実行順依存で単発失敗するケースがあり、同一バイナリで再実行すると PASS する挙動を再確認。
- 残タスク（継続改善）:
  - `diagonalcalc.c` intra 側の重複整理（TEChemi/TETransfer の model分岐統合）。
  - `sz`/`diagonalcalc` characterization の追加拡充（tJ/Kondo/SpinGC）。

## 進捗更新（2026-02-07 00:41 JST）
- `diagonalcalc.c` の intra process 重複整理を実施。
  - `ApplyTEChemiIntraProcess` を簡素化し、
    - `Spin` + general-spin のみ専用経路（`AccumulateTEChemiSpinGeneral`）を維持
    - それ以外は `ApplyTETransferIntraProcess` を再利用
  - これにより TEChemi/TETransfer の intra model分岐重複を統合。
- characterization 拡張を実施。
  - 新規 unit test:
    - `unit_sz_kondo_nconserved_characterization`
  - `Kondo` で `2Sz` 未指定（NConserved）時の `CHECK_Sdim`/`CHECK_Memory` を固定化。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (17/17)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (77/77)
- 備考:
  - `smoke` と `all` を同時実行すると `te_ac_hubbard_square` が干渉で失敗するため、最終検証は単体->smoke->all の順で逐次実行。
- 残タスク（継続改善）:
  - `sz`/`diagonalcalc` characterization の追加拡充（tJ/SpinGC の追加ケース）。

## 進捗更新（2026-02-07 00:45 JST）
- characterization 拡張（SpinGC）を追加実施。
  - 新規 unit test:
    - `unit_diagonalcalc_spingc_characterization`
  - `SpinGC` + `FullDiag` で `zvo_phys.dat` の先頭固有値と `idim_max` を固定化。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (18/18)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (78/78)
- 残タスク（継続改善）:
  - tJ 系 characterization の追加（現状 standard mode の `model="tJ"` は未対応のため、expert mode fixture化が必要）。

## 進捗更新（2026-02-07 00:49 JST）
- tJ 系 characterization を expert mode fixture で追加。
  - 新規 unit test:
    - `unit_sz_tj_expert_characterization`
  - 対応内容:
    - `CalcModel=9`（tJ）固定の `calcmod.def` を fixture 化。
    - `2Sz` あり（tJ）と `2Sz` なし（tJNConserved）を同一テスト内で実行。
    - 検証値:
      - tJ: `sdim=16`, `idim_max=12`
      - tJNConserved: `sdim=16`, `idim_max=32`
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (19/19)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (79/79)
- 残タスク（継続改善）:
  - tJGC (`CalcModel=10`) / 追加 lattice 条件での characterization 拡張。
  - `te_ac_hubbard_square` の並列実行干渉回避（テスト実行設計の明文化）。

## 進捗更新（2026-02-07 00:55 JST）
- tJGC (`CalcModel=10`) の expert mode characterization を追加し、同時に dimension mismatch の根因を修正。
  - `src/check.c`
    - `tJGC` の `idim_max` 計算を `Ne` 依存ループから `Nsite` 依存ループへ変更。
    - `Ne` が未設定（0）の expert fixture でも、`3^Nsite` の次元を正しく計算可能にした。
  - 新規 unit test:
    - `unit_sz_tjgc_expert_characterization`
  - 検証値:
    - `Nsite=4` で `sdim=16`, `idim_max=81`, `Err_sz.dat` 非生成
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)
- 残タスク（継続改善）:
  - `readdef.c` の switch-case 可読性改善（dispatch table 化など）の段階適用。
  - `te_ac_hubbard_square` の並列実行干渉回避（テスト実行設計の明文化）。

## 進捗更新（2026-02-07 00:59 JST）
- 残タスク 2 件を継続実施。
  - `readdef.c` 可読性改善:
    - `ApplyReadNIntModelRules` をモデル分類 + 小関数へ段階分割。
    - `CheckLocSpin` の長い `switch` を helper ベースへ整理。
  - テスト実行設計の明文化:
    - `doc/test_operations.md` に `Known Pitfall` を追加し、
      `te_ac_hubbard_square` の並列干渉と回避・切り分け手順を記載。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)
- 残タスク（継続改善）:
  - `readdef.c` の他大型 `switch`（相互作用条件判定系）の段階整理。
  - 並列干渉を CI 側で回避するための job 分離・排他制御の適用。

## 進捗更新（2026-02-07 01:05 JST）
- 指示順 `2 -> 1` で残タスクを実施。
  - `2` CI 側の干渉回避:
    - `.github/workflows/main.yml` を job 分離。
      - `ctest_unit_smoke`（push/pr）
      - `ctest_full_schedule`（schedule）
    - workflow-level `concurrency` を追加し、同一 ref での重複実行を排他化。
  - `1` `readdef.c` の大型 `switch` 整理:
    - `GetDiagonalInterAll` の off-diagonal model 分岐を helper 化。
    - `ArrangeInterAllOffDiagonal` の model 分岐を helper 化。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)
- 残タスク（継続改善）:
  - `readdef.c` の `GetDiagonalInterAll_simple` / 近傍 duplicate ロジックの統合。
  - CI の重複 step（依存導入/ビルド）を reusable workflow or composite action で共通化。

## 進捗更新（2026-02-07 01:10 JST）
- 残タスク `1,2` を完了。
  - `1` `readdef.c` duplicate 統合:
    - `GetDiagonalInterAll` と `GetDiagonalInterAll_simple` の diagonal 共通処理を helper 化。
    - `GetDiagonalInterAll_simple` の off-diagonal コピーは既存 helper 再利用に統一。
  - `2` CI setup/build 共通化:
    - `.github/actions/bootstrap-build/action.yml` を追加。
    - `.github/workflows/main.yml` は両 job で composite action を利用。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)
- 次の継続改善候補:
  - `GetDiagonalInterAll_simple` 自体の役割整理（必要性の見直し or 呼び出し側の統一）。
  - CI の matrix 最適化（実行時間短縮と失敗時切り分け速度の両立）。

## 進捗更新（2026-02-07 01:14 JST）
- `1,2` の継続改善を追加で実施。
  - `1` 役割整理:
    - `GetDiagonalInterAll` / `GetDiagonalInterAll_simple` の実装を `SplitDiagonalAndOffDiagonalInterAll` へ統合。
    - API互換を保ちつつ、`apply_model_filter` で既存挙動差を維持。
  - `2` matrix 最適化:
    - push/pr を `ctest_unit`（単一構成）と `ctest_smoke`（matrix）へ分離。
    - `unit` 重複実行を削減し、失敗時の切り分け粒度を改善。
- 検証結果（最終）:
  - `ctest --output-on-failure -L unit -j4`: PASS (20/20)
  - `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
  - `ctest --output-on-failure -j4`: PASS (80/80)
- 次の継続改善候補:
  - `GetDiagonalInterAll_simple` 呼び出し側の統一方針（完全統合するか、軽量経路として残すか）の決定。
  - schedule matrix の実行コスト最適化（必要 coverage を保ったままの縮約）。
