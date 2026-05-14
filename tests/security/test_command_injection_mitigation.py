import sys
import os
import subprocess
import pytest

# Add scripts directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../scripts')))

def test_sonar_helper_no_shell_true():
    with open('scripts/sonar_helper.py', 'r') as f:
        content = f.read()
    assert 'shell=True' not in content, "shell=True should not be used in sonar_helper.py"

def test_zenith_audit_scribe_no_shell_true():
    with open('scripts/zenith_audit_scribe.py', 'r') as f:
        content = f.read()
    assert 'shell=True' not in content, "shell=True should not be used in zenith_audit_scribe.py"

if __name__ == "__main__":
    pytest.main([__file__])
