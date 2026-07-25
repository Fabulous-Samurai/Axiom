## 2026-07-25 - Prevent CI Supply Chain Attacks
**Vulnerability:** CI workflows used outdated pinned GitHub Action commit SHAs and an unmaintained third-party pre-commit tool (`pwaller/pre-commit-clang-format`).
**Learning:** Hardcoded, outdated commit SHAs and unmaintained third-party dependencies can become attack vectors if a repository is compromised or deprecated. Pinned SHAs must be routinely audited and updated to valid commit SHAs corresponding to the desired newer tags (via `git ls-remote`). Unmaintained external tools should be replaced with official mirrors.
**Prevention:** Use maintained official mirrors (`pre-commit/mirrors-clang-format`) and keep pinned workflow SHAs continuously up to date.
