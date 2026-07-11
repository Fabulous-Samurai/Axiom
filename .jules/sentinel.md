## 2026-07-11 - Prevent Sandbox Escape via eval()
**Vulnerability:** Arbitrary code execution vulnerability via unchecked eval() in the sandbox subprocess, allowing access to the file system and potential system compromise.
**Learning:** Using eval() on user input is inherently dangerous, even in a separate subprocess, unless the execution environment (globals and locals) is explicitly restricted by removing __builtins__. Subprocess isolation is not enough if the evaluated code can access the python os module.
**Prevention:** Always explicitly restrict the global and local scopes when using eval(), such as by passing `{'__builtins__': {}}` to prevent access to built-in functions like `__import__`.
