import pytest
from scripts.sandbox import run_isolated_expression

def test_safe_math():
    assert run_isolated_expression("2 + 2") == "4"
    assert run_isolated_expression("2 * 3") == "6"

def test_prevent_import():
    res = run_isolated_expression("__import__('os').listdir('.')")
    assert "Error:" in res

def test_prevent_exec():
    res = run_isolated_expression("exec('print(1)')")
    assert "Error:" in res

def test_timeout():
    print("[INFO] Testing timeout mechanism...")
    res = run_isolated_expression("__import__('time').sleep(10)")
    is_timeout = "Error:" in res
    assert is_timeout, f"Expected error or timeout, got: {res}"

if __name__ == '__main__':
    pytest.main(['-v', 'tests/sandbox_security_test.py'])
