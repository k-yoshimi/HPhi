Date: 2026-02-07
Time: 11:59:08 JST

# Refactoring Session 054 (optional full CI on push by commit message)

## 仕様変更
- `.github/workflows/main.yml`
  - `ctest_full_schedule` の起動条件を更新。
  - 既存の `schedule` 実行に加えて、`push` 時にコミットメッセージへ `[full-ci]` を含む場合にも実行。
  - 条件式:
    - `github.event_name == 'schedule'`
    - `|| (github.event_name == 'push' && contains(join(github.event.commits.*.message, ' '), '[full-ci]'))`

## 使い方（コミットメッセージ）
- 通常の push（full を回さない）:
  - 例: `refactor: cleanup readdef helpers`
- full も回したい push:
  - 例: `refactor: validate ci partitioning [full-ci]`

## 補足
- `push` に含まれるコミット群のメッセージを連結して判定するため、同一 push 内のいずれかのコミットに `[full-ci]` があれば `ctest_full_schedule` が起動する。
