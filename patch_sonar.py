with open(".github/workflows/axiom_zenith_ci.yml", "r") as f:
    content = f.read()

# Try to use pip show to find actual resolved versions in active bash session
# pre-commit version found: 4.6.2
# nanobind version found: 3.1.0
# pytest version found: 9.1.1
# numpy version found: 2.5.3
# matplotlib version found: 3.11.2

# Let's fix the specific warnings.
# SonarCloud is complaining about the pip install commands missing a --no-cache-dir or similar security best practices for pip,
# or perhaps it wants a requirements file. Let's see the specific SonarCloud rule for AaEZTmh2uQQGkwG8MNTK if possible, or
# follow best practices for pip install in CI:
# 1. Use --no-cache-dir to avoid caching issues and reduce image size
# 2. Use --disable-pip-version-check
# 3. Consider not hardcoding the version here but instead use a requirements file. However memory says "ensure all Python dependencies installed via `pip` are strictly pinned to a specific version (e.g., `python -m pip install pytest==8.2.2`)."
# Let's double check if we missed any pip install commands or if there's a better way to satisfy Sonar.
# Wait, memory explicitly states: "To fix SonarCloud Security Rating (Quality Gate) failures related to CI workflows, ensure all Python dependencies installed via `pip` are strictly pinned to a specific version (e.g., `python -m pip install pytest==8.2.2`)."

# Let's inspect line 25, 35, 56, 65, 102, etc.
# Line 25: python -m pip install pre-commit==4.6.2
# Wait! We need to make sure we didn't leave any other `pip install` without version.
# Are there any other pip installs? Let's check `grep pip .github/workflows/axiom_zenith_ci.yml`
