// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "../include/python_parser.h"

namespace AXIOM {

PythonParser::PythonParser(PythonEngine* engine, PythonMode mode) : python_engine_(engine), mode_(mode) {
}

EngineResult PythonParser::ParseAndExecute(std::string_view input) noexcept {
    if (!python_engine_) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }
    if (!python_engine_->IsInitialized()) {
        EngineResult res;
        res.error = EngineErrorResult{CalcErr::OperationNotFound};
        return res;
    }

    std::string processed_input;

    switch (mode_) {
        case PythonMode::Interactive:
            processed_input = HandleInteractiveMode(input);
            break;
        case PythonMode::NumPy:
            processed_input = HandleNumPyMode(input);
            break;
        case PythonMode::SciPy:
            processed_input = HandleSciPyMode(input);
            break;
        case PythonMode::Matplotlib:
            processed_input = HandleMatplotlibMode(input);
            break;
        case PythonMode::Pandas:
            processed_input = HandlePandasMode(input);
            break;
        case PythonMode::SymPy:
            processed_input = HandleSymPyMode(input);
            break;
        default:
            processed_input = std::string(input);
            break;
    }

    // Execute the processed Python code
    return python_engine_->EvaluatePython(processed_input);
}

std::string PythonParser::HandleInteractiveMode(std::string_view input) {
    // Interactive mode - execute Python code as-is
    return std::string(input);
}

static void ReplaceFunctionCalls(std::string& processed, const AXIOM::FixedVector<std::string_view, 256>& funcs, std::string_view prefix, char suffix) {
    for (const auto& func : funcs) {
        if (processed.find(func) != std::string::npos) {
            std::string pattern = std::string(func);
            pattern += suffix;
            std::string replacement = std::string(prefix);
            replacement += std::string(func);
            replacement += suffix;
            size_t pos = processed.find(pattern);
            while (pos != std::string::npos) {
                processed.replace(pos, pattern.length(), replacement);
                pos = processed.find(pattern, pos + replacement.length());
            }
        }
    }
}

std::string PythonParser::HandleNumPyMode(std::string_view input) {
    // NumPy mode - automatically add np. prefix to numpy functions
    std::string processed{input};

    // Add common NumPy function prefixes - Use FixedVector with explicit constructor
    AXIOM::FixedVector<std::string_view, 256> numpy_funcs({
        "array", "zeros", "ones", "linspace", "arange", "reshape",
        "dot", "cross", "sum", "mean", "std", "min", "max", "sqrt",
        "exp", "log", "sin", "cos", "tan", "pi", "e"
    });

    ReplaceFunctionCalls(processed, numpy_funcs, "np.", '(');

    return processed;
}

std::string PythonParser::HandleSciPyMode(std::string_view input) {
    // SciPy mode - add scipy prefixes
    std::string processed = HandleNumPyMode(input); // Include NumPy support

    AXIOM::FixedVector<std::string_view, 256> scipy_funcs({
        "integrate", "optimize", "linalg", "stats", "special", "fft"
    });

    ReplaceFunctionCalls(processed, scipy_funcs, "sp.", '.');

    return processed;
}

std::string PythonParser::HandleMatplotlibMode(std::string_view input) {
    // Matplotlib mode - add plotting shortcuts
    std::string processed = HandleNumPyMode(input); // Include NumPy support

    AXIOM::FixedVector<std::string_view, 256> plt_funcs({
        "plot", "scatter", "bar", "hist", "show", "figure", "subplot",
        "xlabel", "ylabel", "title", "legend", "grid", "savefig"
    });

    ReplaceFunctionCalls(processed, plt_funcs, "plt.", '(');

    return processed;
}

std::string PythonParser::HandlePandasMode(std::string_view input) {
    // Pandas mode - add pandas shortcuts
    std::string processed = HandleNumPyMode(input); // Include NumPy support

    AXIOM::FixedVector<std::string_view, 256> pd_funcs({
        "DataFrame", "Series", "read_csv", "read_excel", "read_json"
    });

    ReplaceFunctionCalls(processed, pd_funcs, "pd.", '(');

    return processed;
}

std::string PythonParser::HandleSymPyMode(std::string_view input) {
    // SymPy mode - add symbolic math shortcuts
    std::string processed{input};

    AXIOM::FixedVector<std::string_view, 256> sympy_funcs({
        "Symbol", "symbols", "diff", "integrate", "solve", "expand",
        "factor", "simplify", "limit", "series"
    });

    ReplaceFunctionCalls(processed, sympy_funcs, "sp.", '(');

    return processed;
}

} // namespace AXIOM
