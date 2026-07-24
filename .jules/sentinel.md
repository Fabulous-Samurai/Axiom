## 2026-07-24 - Supply Chain Security Improvements
**Vulnerability:** Invalid commit hashes pinned in CI actions and use of unmaintained 3rd-party pre-commit hooks.
**Learning:** Hardcoding or not verifying correct commit SHAs for dependencies can lead to executing compromised code, and relying on abandoned community projects increases the surface area for unpatched vulnerabilities.
**Prevention:** Always verify the correct commit SHAs using `git ls-remote` for the intended tag versions, and prefer official/maintained mirrors or actions over random 3rd-party repositories.
