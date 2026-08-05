## 2026-08-05 - Supply Chain Security Enhancements
**Vulnerability:** Invalid commit SHA for setup-java action and use of unmaintained/deleted pre-commit hook (pwaller/pre-commit-clang-format).
**Learning:** Using incorrect SHAs can lead to action resolution failures in CI, and relying on unmaintained third-party forks for critical tools increases the risk of supply chain attacks.
**Prevention:** Always verify commit SHAs against the official repository tags. Use officially maintained mirrors (e.g., pre-commit/mirrors-clang-format) instead of obscure forks.
