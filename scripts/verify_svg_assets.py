import os
import re

def verify_svg(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Check for viewBox (required for proper scaling)
    if 'viewBox' not in content:
        print(f"[FAIL] {file_path}: Missing viewBox attribute.")
        return False

    # Check for hardcoded dimensions that might cause artifacting if not handled correctly
    # (Though width/height are fine if viewBox is present)

    # Check if 'currentColor' is used for theme compatibility
    if 'fill="currentColor"' not in content and 'stroke="currentColor"' not in content:
        # Some icons might have hardcoded colors like #7DD3FC, check if that's intended
        if '#7DD3FC' in content:
            pass # Axiom brand color is okay
        else:
            print(f"[WARN] {file_path}: Does not use currentColor or brand color (#7DD3FC).")

    print(f"[PASS] {file_path}: Vector integrity verified.")
    return True

icons_dir = "products/studio/ui/assets/icons"
if os.path.exists(icons_dir):
    for icon in os.listdir(icons_dir):
        if icon.endswith(".svg"):
            verify_svg(os.path.join(icons_dir, icon))
else:
    print(f"Directory not found: {icons_dir}")
