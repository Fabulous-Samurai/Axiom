with open(".github/workflows/axiom_zenith_ci.yml", "r") as f:
    content = f.read()
content = content.replace(
    "uses: actions/setup-java@99b807301553181616292f2c82202300392f0b57",
    "uses: actions/setup-java@6a0805fcefea3d4657a47ac4c165951e33482018"
)
with open(".github/workflows/axiom_zenith_ci.yml", "w") as f:
    f.write(content)
