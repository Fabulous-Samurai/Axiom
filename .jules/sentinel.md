## 2026-10-07 - [Secure Python Sandbox AST Whitelisting]
**Vulnerability:** [Arbitrary code execution via unrestricted eval() in sandbox evaluation]
**Learning:** [Disabling builtins is insufficient; attackers can use MRO traversal. A strict AST-based whitelisting approach is required to safely evaluate math expressions without allowing arbitrary imports or resource exhaustion (e.g. sequence repetition).]
**Prevention:** [Use ast.NodeVisitor to strictly whitelist allowed operations, functions, and names, explicitly enforcing type checks on operators like ast.Mult to prevent memory exhaustion.]
