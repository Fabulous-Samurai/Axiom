## 2026-07-27 - Prevent Arbitrary Code Execution in Sandbox eval()
**Vulnerability:** The sandbox evaluator used an unrestricted eval(), allowing users to bypass standard OS restrictions and execute arbitrary Python code.
**Learning:** External restrictions like seccomp or AppContainer are not a substitute for restricting the execution context itself; relying on subprocess isolation is insufficient without sandboxing the eval() environment.
**Prevention:** Always pass a whitelist for globals/locals, particularly specifying an empty or safe __builtins__ dictionary when using eval().
