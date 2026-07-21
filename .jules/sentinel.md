## 2026-07-21 - Unmaintained Dependency Replacement
**Vulnerability:** Found unmaintained third-party action `pwaller/pre-commit-clang-format` which presents a supply chain security risk.
**Learning:** Legacy repositories and third-party tools should be routinely checked for maintenance status.
**Prevention:** Always default to official, well-maintained repositories (like `pre-commit/mirrors-clang-format`) or pin actions to a specific SHA.
