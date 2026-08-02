## 2026-08-02 - Securing CI Supply Chain
**Vulnerability:** CI/CD pipeline dependency failure due to an unmaintained, deleted third-party tool (`pwaller/pre-commit-clang-format`).
**Learning:** Relying on unverified or single-maintainer third-party repositories for code execution during CI poses a significant supply chain security risk (e.g., repository hijacking or deletion breaking pipelines).
**Prevention:** Always use official, organization-backed mirrors (like `pre-commit/mirrors-clang-format`) or explicitly pin dependencies to verified commit SHAs to prevent supply chain attacks and pipeline fragility.
