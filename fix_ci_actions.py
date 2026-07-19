import os

with open(".github/workflows/axiom_zenith_ci.yml", "r") as f:
    content = f.read()

content = content.replace(
    "actions/checkout@692973e3d937129bcbf40652eb9f2f61becf3332",
    "actions/checkout@11bd71901bbe5b1630ceea73d27597364c9af683"
)

content = content.replace(
    "actions/setup-python@39cd14951b08e74b54015e9e001cdefcf80e669f",
    "actions/setup-python@0b93645e9fea7318ecaed2b359559ac225c90a2b"
)

with open(".github/workflows/axiom_zenith_ci.yml", "w") as f:
    f.write(content)
