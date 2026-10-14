## YYYY-MM-DD - [Title]
**Vulnerability:** Arbitrary Code Execution via python eval()
**Learning:** Python eval() even with input restrictions can be dangerous because built-in functions allow escaping out to os modules via __import__ or executing malicious payloads to exahaust memory / cpu like `10**1000`.
**Prevention:** Avoid `eval` for parsing user input strings. Use `ast.parse()` combined with `ast.NodeVisitor` to safely evaluate expressions and strictly whitelist permitted operations and types. Ensure operand limits or strict type-checking is applied for operations like exponentiation.
