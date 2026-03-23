import sys
from unittest.mock import MagicMock
import unittest

# Mock necessary libraries
mock_np = MagicMock()
mock_np.sin.side_effect = lambda x: f"sin({x})"
mock_np.cos.side_effect = lambda x: f"cos({x})"
mock_np.sqrt.side_effect = lambda x: f"sqrt({x})"
mock_np.pi = 3.14159

sys.modules['numpy'] = mock_np
sys.modules['matplotlib'] = MagicMock()
sys.modules['matplotlib.pyplot'] = MagicMock()
sys.modules['mpl_toolkits.mplot3d'] = MagicMock()
sys.modules['matplotlib.animation'] = MagicMock()
sys.modules['matplotlib.widgets'] = MagicMock()
sys.modules['tkinter'] = MagicMock()
sys.modules['tkinter.ttk'] = MagicMock()
sys.modules['tkinter.messagebox'] = MagicMock()

from tools.visualization.advanced_3d_visualization import SafeMathEvaluator

class TestSafeMathEvaluator(unittest.TestCase):
    def setUp(self):
        self.safe_dict = {
            'sin': mock_np.sin,
            'cos': mock_np.cos,
            'sqrt': mock_np.sqrt,
            'pi': mock_np.pi,
            'x': 10,
            'y': 20,
            't': 0.5
        }

    def test_basic_arithmetic(self):
        self.assertEqual(SafeMathEvaluator.evaluate("1 + 2", self.safe_dict), 3)
        self.assertEqual(SafeMathEvaluator.evaluate("10 - 5", self.safe_dict), 5)
        self.assertEqual(SafeMathEvaluator.evaluate("2 * 3", self.safe_dict), 6)
        self.assertEqual(SafeMathEvaluator.evaluate("10 / 2", self.safe_dict), 5.0)
        self.assertEqual(SafeMathEvaluator.evaluate("2 ** 3", self.safe_dict), 8)
        self.assertEqual(SafeMathEvaluator.evaluate("-x", self.safe_dict), -10)

    def test_functions_and_variables(self):
        self.assertEqual(SafeMathEvaluator.evaluate("sin(x)", self.safe_dict), "sin(10)")
        self.assertEqual(SafeMathEvaluator.evaluate("sqrt(x**2 + y**2)", self.safe_dict), "sqrt(500)")

    def test_rejection(self):
        # Malicious attempts
        with self.assertRaises(ValueError):
            SafeMathEvaluator.evaluate("().__class__", self.safe_dict)

        with self.assertRaises(ValueError):
             # SafeMathEvaluator handles only expression nodes within tree.body
             # [x for x in (1,2,3)] is a ListComp node which is not whitelisted
             SafeMathEvaluator.evaluate("[x for x in (1,2,3)]", self.safe_dict)

        with self.assertRaises(ValueError):
            SafeMathEvaluator.evaluate("open('/etc/passwd').read()", self.safe_dict)

    def test_nested_expressions(self):
         self.assertEqual(SafeMathEvaluator.evaluate("sin(sqrt(x**2 + y**2))", self.safe_dict), "sin(sqrt(500))")

if __name__ == "__main__":
    unittest.main()
