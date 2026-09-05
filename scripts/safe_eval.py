import ast
import operator

def evaluate(expr_str):
    ALLOWED_BUILTINS = {
        'abs': abs, 'min': min, 'max': max, 'round': round, 'sum': sum,
        'len': len, 'bool': bool, 'int': int, 'float': float, 'str': str,
        'dict': dict, 'list': list, 'set': set, 'tuple': tuple,
        'True': True, 'False': False, 'None': None
    }

    ALLOWED_OPERATORS = {
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod, ast.Pow: operator.pow, ast.BitXor: operator.xor,
        ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
        ast.USub: operator.neg, ast.UAdd: operator.pos,
        ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt,
        ast.LtE: operator.le, ast.Gt: operator.gt, ast.GtE: operator.ge,
        ast.In: lambda x, y: x in y, ast.NotIn: lambda x, y: x not in y,
        ast.Not: operator.not_
    }

    def _eval(node):
        if isinstance(node, ast.Expression):
            return _eval(node.body)
        elif isinstance(node, ast.Constant):
            return node.value
        elif hasattr(ast, 'Num') and isinstance(node, ast.Num): return node.n
        elif hasattr(ast, 'Str') and isinstance(node, ast.Str): return node.s
        elif hasattr(ast, 'NameConstant') and isinstance(node, ast.NameConstant): return node.value
        elif isinstance(node, ast.UnaryOp):
            if type(node.op) in ALLOWED_OPERATORS:
                return ALLOWED_OPERATORS[type(node.op)](_eval(node.operand))
            raise ValueError(f"Unsupported unary operator: {type(node.op)}")
        elif isinstance(node, ast.BinOp):
            if type(node.op) in ALLOWED_OPERATORS:
                return ALLOWED_OPERATORS[type(node.op)](_eval(node.left), _eval(node.right))
            raise ValueError(f"Unsupported binary operator: {type(node.op)}")
        elif isinstance(node, ast.BoolOp):
            if type(node.op) == ast.And:
                for val in node.values:
                    res = _eval(val)
                    if not res: return res
                return res
            elif type(node.op) == ast.Or:
                for val in node.values:
                    res = _eval(val)
                    if res: return res
                return res
            raise ValueError(f"Unsupported boolean operator: {type(node.op)}")
        elif isinstance(node, ast.Compare):
            left = _eval(node.left)
            for op, comp in zip(node.ops, node.comparators):
                if type(op) in ALLOWED_OPERATORS:
                    right = _eval(comp)
                    if not ALLOWED_OPERATORS[type(op)](left, right):
                        return False
                    left = right
                else:
                    raise ValueError(f"Unsupported comparison operator: {type(op)}")
            return True
        elif isinstance(node, ast.Name):
            if node.id in ALLOWED_BUILTINS:
                return ALLOWED_BUILTINS[node.id]
            raise ValueError(f"Disallowed name: {node.id}")
        elif isinstance(node, ast.Call):
            func = _eval(node.func)
            args = [_eval(arg) for arg in node.args]
            kwargs = {kw.arg: _eval(kw.value) for kw in node.keywords if kw.arg}
            return func(*args, **kwargs)
        elif isinstance(node, ast.List):
            return [_eval(elt) for elt in node.elts]
        elif isinstance(node, ast.Tuple):
            return tuple(_eval(elt) for elt in node.elts)
        elif isinstance(node, ast.Set):
            return {_eval(elt) for elt in node.elts}
        elif isinstance(node, ast.Dict):
            return {_eval(k): _eval(v) for k, v in zip(node.keys, node.values)}
        elif isinstance(node, ast.Subscript):
            return _eval(node.value)[_eval(node.slice)]
        elif isinstance(node, ast.IfExp):
            return _eval(node.body) if _eval(node.test) else _eval(node.orelse)
        elif hasattr(ast, 'Index') and isinstance(node, ast.Index): return _eval(node.value)
        elif isinstance(node, ast.Slice):
            lower = _eval(node.lower) if node.lower else None
            upper = _eval(node.upper) if node.upper else None
            step = _eval(node.step) if node.step else None
            return slice(lower, upper, step)
        elif isinstance(node, ast.Attribute):
            raise ValueError(f"Attribute access is disallowed: {node.attr}")
        else:
            raise ValueError(f"Unsupported AST node: {type(node)}")

    return _eval(ast.parse(expr_str, mode='eval'))
