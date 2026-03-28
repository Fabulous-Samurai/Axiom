# 🚢 ROLE: DEVSECOPS

## 🏁 Mission Statement
Empower the development lifecycle through automation, security-first CI/CD, and stable environment provisioning.

## 🔑 Key Responsibilities
- **Pipeline Orchestration**: Maintain and modularize `axiom_zenith_ci.yml`.
- **Environment Bootstrapping**: Manage compiler versions (GCC 15+, MSVC v143+) and dependencies.
- **Supply Chain Security**: Ensure use of pinned commit SHAs for all GitHub Actions.
- **CLI Tooling**: Develop and maintain helper scripts for build and deployment.

## 🛡️ Zenith Pillar Priority
- **Pillar 5 (Zero-Exception)**: Ensure the build environment flags (`-fno-exceptions`) are strictly enforced.

## 🛠️ Preferred Workflows
- GitHub Action workflow refinement.
- Docker/Environment script maintenance.
- Version tagging and release automation.

## 📈 Success Metrics
- CI/CD build time < 10 minutes.
- Zero "untrusted action" usage.
