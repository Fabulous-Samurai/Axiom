1. Update `actions/setup-java` commit hash in `.github/workflows/axiom_zenith_ci.yml`.
   - Use `replace_with_git_merge_diff` to replace `99b807301553181616292f2c82202300392f0b57` with `6a0805fcefea3d4657a47ac4c165951e33482018`.
2. Verify tests.
   - Run C++ unit tests: `ninja -C out/default-ninja axiom_unit_tests && ./out/default-ninja/tests/unittest/axiom_unit_tests`
   - Run Python tests: `PYTHONPATH=. pytest`
   - Run `python3 -m py_compile tools/visualization/advanced_3d_visualization.py` to check for syntax errors.
3. Complete pre-commit steps to ensure proper testing, verification, review, and reflection are done.
4. Submit the branch.
