## 2026-07-20 - Supply Chain Security Improvements
**Vulnerability:** Use of unmaintained pre-commit hooks (pwaller/pre-commit-clang-format) and outdated/invalid GitHub Action pinned SHAs that cause deprecation warnings and potential integrity risks.
**Learning:** Supply chain components must be regularly updated to verified versions and exact commit SHAs to prevent upstream compromises and maintain CI stability.
**Prevention:** Regularly audit third-party dependencies, replace unmaintained tools with official mirrors, and verify all pinned GitHub Action SHAs against upstream tags using git ls-remote.
