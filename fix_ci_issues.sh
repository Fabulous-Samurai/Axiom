sed -i -E 's|actions/checkout@[a-f0-9]+|actions/checkout@11bd71901bbe5b1630ceea73d27597364c9af683|g' .github/workflows/axiom_zenith_ci.yml .github/workflows/pull_sonar_issues.yml
sed -i -E 's|actions/setup-python@[a-f0-9]+|actions/setup-python@0b93645e9fea7318ecaed2b359559ac225c90a2b|g' .github/workflows/axiom_zenith_ci.yml .github/workflows/pull_sonar_issues.yml
