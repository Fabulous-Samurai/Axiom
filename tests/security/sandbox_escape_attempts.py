import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../scripts')))
from sandbox import run_isolated_expression

def test_ast_escape():
    # Try different escape vectors
    res = run_isolated_expression("__import__('os').system('ls')")
    assert "Function calls are explicitly disallowed" in res

    res = run_isolated_expression("open('/etc/passwd')")
    assert "Function calls are explicitly disallowed" in res

    res = run_isolated_expression("().__class__.__base__.__subclasses__")
    assert "Attribute access is explicitly disallowed" in res

