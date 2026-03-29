// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "linear_system_parser.h"
#include "string_helpers.h"
#ifdef ENABLE_EIGEN
#include "eigen_engine.h"
#endif
#include "arena_allocator.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <iostream>
#include <cctype>
#include <charconv>
#include "arena_allocator.h"

namespace AXIOM
{

    // =========================================================
    // LEXER (TOKENIZER) IMPLEMENTATION
    // =========================================================
    namespace
    {
        enum class TokenType
        {
            LBracket,
            RBracket,
            Semicolon,
            Comma,
            Number,
            End
        };

        struct Token
        {
            TokenType type;
            std::string value;
        };

        class MatrixLexer
        {
        public:
            explicit MatrixLexer(std::string_view input) noexcept : input_(input), pos_(0) {}

            Token NextToken() noexcept
            {
                SkipWhitespace();
                if (pos_ >= input_.length())
                    return {TokenType::End, ""};

                char current = input_[pos_];

                if (current == '[')
                {
                    pos_++;
                    return {TokenType::LBracket, "["};
                }
                if (current == ']')
                {
                    pos_++;
                    return {TokenType::RBracket, "]"};
                }
                if (current == ';')
                {
                    pos_++;
                    return {TokenType::Semicolon, ";"};
                }
                if (current == ',')
                {
                    pos_++;
                    return {TokenType::Comma, ","};
                }

                if (std::isdigit(static_cast<unsigned char>(current)) || current == '-' || current == '.')
                {
                    size_t start = pos_;
                    if (current == '-')
                    {
                        pos_++;
                    }

                    bool has_decimal = false;
                    while (pos_ < input_.length())
                    {
                        char c = input_[pos_];
                        if (std::isdigit(static_cast<unsigned char>(c)))
                        {
                            pos_++;
                        }
                        else if (c == '.' && !has_decimal)
                        {
                            has_decimal = true;
                            pos_++;
                        }
                        else
                            break;
                    }

                    std::string_view num_sv = input_.substr(start, pos_ - start);
                    if (num_sv == "-" || num_sv == ".")
                        return {TokenType::End, ""};
                    return {TokenType::Number, std::string(num_sv)};
                }

                pos_++;
                return NextToken();
            }

        private:
            std::string_view input_;
            size_t pos_;

            void SkipWhitespace() noexcept
            {
                while (pos_ < input_.length() && std::isspace(static_cast<unsigned char>(input_[pos_])))
                    pos_++;
            }
        };
    }

    // =========================================================
    // LINEAR SYSTEM PARSER IMPLEMENTATION
    // =========================================================

    LinearSystemParser::LinearSystemParser()
    {
        RegisterCommands();
    }

    void LinearSystemParser::RegisterCommands()
    {
        command_registry_.emplace_back(CommandEntry{"qr", [this](std::string_view s) noexcept
                                     { return HandleQR(s); }, "Performs QR Decomposition"});
        command_registry_.emplace_back(CommandEntry{"ortho", [this](std::string_view s) noexcept
                                     { return HandleQR(s); }, "Performs QR Decomposition"});
        command_registry_.emplace_back(CommandEntry{"eigen", [this](std::string_view s) noexcept
                                     { return HandleEigen(s); }, "Computes Eigenvalues and Eigenvectors"});
        command_registry_.emplace_back(CommandEntry{"cramer", [this](std::string_view s) noexcept
                                     { return HandleCramer(s); }, "Solves system using Cramer's Rule"});
        command_registry_.emplace_back(CommandEntry{"solve", [this](std::string_view s) noexcept
                                     { return HandleSolve(s); }, "Solves linear system Ax=b"});
    }

    EngineResult LinearSystemParser::ParseAndExecute(std::string_view input) noexcept {
        for (const auto &entry : command_registry_)
        {
            if (input.find(entry.command) == 0)
            {
                return entry.handler(input);
            }
        }
        return HandleDefaultSolve(input);
    }

    EngineResult LinearSystemParser::HandleQR(std::string_view input) const noexcept
    {
        auto extract_matrix_string = [input]() -> std::string_view
        {
            size_t matrix_start = input.find_first_of("0123456789-[");
            if (matrix_start == std::string_view::npos)
                return std::string_view();
            return input.substr(matrix_start);
        };

        std::string_view matrix_str = extract_matrix_string();
        if (matrix_str.empty())
            return CreateErrorResult(LinAlgErr::ParseError);

        auto A = ParseMatrixString(matrix_str);
        if (A.empty())
            return CreateErrorResult(LinAlgErr::ParseError);
        if (A.size() < A[0].size())
            return CreateErrorResult(LinAlgErr::MatrixMismatch);

        auto [Q, R] = GramSchmidt(A);
        if (Q.empty())
            return CreateErrorResult(LinAlgErr::NoSolution);

        // Convert ArenaVector<ArenaVector<double>> to Matrix (std::vector<std::vector<double>>) for CreateSuccessResult
        Matrix result_matrix;
        for (const auto& row : Q) {
            Vector result_row;
            for (double val : row) result_row.push_back(val);
            result_matrix.push_back(std::move(result_row));
        }

        return CreateSuccessResult(std::move(result_matrix));
    }


    EngineResult LinearSystemParser::HandleEigen(std::string_view input) const noexcept
    {
        auto extract_matrix_string = [input]() -> std::string_view
        {
            size_t matrix_start = input.find_first_of("0123456789-[");
            if (matrix_start == std::string_view::npos)
                return std::string_view();
            return input.substr(matrix_start);
        };

        std::string_view matrix_str = extract_matrix_string();
        if (matrix_str.empty())
            return CreateErrorResult(LinAlgErr::ParseError);

        auto A = ParseMatrixString(matrix_str);
        if (A.empty() || A.size() != A[0].size())
            return CreateErrorResult(LinAlgErr::MatrixMismatch);

        auto [eigenValues, eigenVectors] = ComputeEigenvalues(A, 1);

        Vector result_vec;
        for (double val : eigenValues) result_vec.push_back(val);

        return CreateSuccessResult(std::move(result_vec));
    }

    EngineResult LinearSystemParser::HandleCramer(std::string_view input) const noexcept
    {
        std::string_view equation{input.substr(std::min<size_t>(6, input.size()))};
        ArenaVector<ArenaVector<double>> A;
        ArenaVector<double> b;

        if (!ParseLinearSystem(equation, A, b))
            return CreateErrorResult(LinAlgErr::ParseError);
        if (A.size() != A[0].size() || A.size() != b.size())
            return CreateErrorResult(LinAlgErr::MatrixMismatch);

        auto solution = CramersRule(A, b);
        if (solution.has_value()) {
            Vector result_vec;
            for (double val : solution.value()) result_vec.push_back(val);
            return CreateSuccessResult(std::move(result_vec));
        } else {
            return CreateErrorResult(LinAlgErr::NoSolution);
        }
    }

    namespace {
        std::pair<std::string_view, size_t> extract_bracket_block(std::string_view s, size_t start_pos) noexcept
        {
            size_t start = s.find('[', start_pos);
            if (start == std::string_view::npos)
                return {"", std::string_view::npos};
            int depth = 0;
            for (size_t i = start; i < s.size(); ++i)
            {
                if (s[i] == '[')
                    depth++;
                else if (s[i] == ']')
                {
                    depth--;
                    if (depth == 0)
                        return {s.substr(start, i - start + 1), i + 1};
                }
            }
            return {"", std::string_view::npos};
        }
    }

    EngineResult LinearSystemParser::HandleSolve(std::string_view input) const noexcept
    {
        std::string_view content = input.substr(std::min<size_t>(5, input.size()));

        std::string processed; // Temporary copy needed to remove spaces if we don't have a view-based space-ignoring parser
        processed.reserve(content.size());
        for (char c : content)
        {
            if (c != ' ')
                processed += c;
        }

        auto [matrix_str, after_matrix] = extract_bracket_block(processed, 0);
        if (matrix_str.empty() || after_matrix == std::string_view::npos)
            return CreateErrorResult(LinAlgErr::ParseError);

        auto [vec_str, _after_vec] = extract_bracket_block(processed, after_matrix);
        if (vec_str.empty())
            return CreateErrorResult(LinAlgErr::ParseError);

        auto A = ParseMatrixString(matrix_str);
        if (A.empty() || A[0].empty())
            return CreateErrorResult(LinAlgErr::ParseError);

        auto b_matrix = ParseMatrixString(vec_str);
        if (b_matrix.empty() || b_matrix[0].empty())
            return CreateErrorResult(LinAlgErr::ParseError);

        ArenaVector<double> b;
        if (b_matrix.size() == 1)
        {
            b = b_matrix[0];
        }
        else if (b_matrix[0].size() == 1)
        {
            for (const auto &row : b_matrix)
                b.push_back(row[0]);
        }
        else
        {
            return CreateErrorResult(LinAlgErr::ParseError);
        }

        if (A.size() != A[0].size() || A.size() != b.size())
            return CreateErrorResult(LinAlgErr::MatrixMismatch);

        auto solution = solve_linear_system(A, b);
        if (!solution.empty())
        {
            Vector result_vec;
            for (double val : solution) result_vec.push_back(val);
            return CreateSuccessResult(std::move(result_vec));
        }
        else
        {
            return CreateErrorResult(LinAlgErr::NoSolution);
        }
    }

    EngineResult LinearSystemParser::HandleDefaultSolve(std::string_view input) const noexcept
    {
        ArenaVector<ArenaVector<double>> A;
        ArenaVector<double> b;

        if (!ParseLinearSystem(input, A, b))
            return CreateErrorResult(LinAlgErr::ParseError);
        if (A.size() == 0 || A.size() != b.size())
            return CreateErrorResult(LinAlgErr::MatrixMismatch);

        auto solution = solve_linear_system(A, b);
        if (!solution.empty()) {
            Vector result_vec;
            for (double val : solution) result_vec.push_back(val);
            return CreateSuccessResult(std::move(result_vec));
        } else {
            return CreateErrorResult(LinAlgErr::NoSolution);
        }
    }

    ArenaVector<ArenaVector<double>> LinearSystemParser::MultiplyMatrices(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<ArenaVector<double>> &B) const noexcept
    {
        if (A.empty() || B.empty() || A[0].size() != B.size())
            return {};

        const int n = static_cast<int>(A.size()), m = static_cast<int>(B[0].size()), p = static_cast<int>(B.size());
        ArenaVector<ArenaVector<double>> C(n, ArenaVector<double>(m, 0.0));
        const int BLOCK_SIZE = 64;

        for (int ii = 0; ii < n; ii += BLOCK_SIZE)
        {
            for (int jj = 0; jj < m; jj += BLOCK_SIZE)
            {
                for (int kk = 0; kk < p; kk += BLOCK_SIZE)
                {
                    int i_end = std::min(ii + BLOCK_SIZE, n);
                    int j_end = std::min(jj + BLOCK_SIZE, m);
                    int k_end = std::min(kk + BLOCK_SIZE, p);

                    for (int i = ii; i < i_end; i++)
                    {
                        for (int j = jj; j < j_end; j++)
                        {
                            double sum = C[i][j];
                            for (int k = kk; k < k_end; k++)
                                sum += A[i][k] * B[k][j];
                            C[i][j] = sum;
                        }
                    }
                }
            }
        }
        return C;
    }

    ArenaVector<ArenaVector<double>> LinearSystemParser::CreateIdentityMatrix(int n) const noexcept
    {
        ArenaVector<ArenaVector<double>> I(n, ArenaVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i)
            I[i][i] = 1.0;
        return I;
    }

    ArenaVector<double> LinearSystemParser::GetDiagonal(const ArenaVector<ArenaVector<double>> &A) const noexcept
    {
        ArenaVector<double> diag;
        for (size_t i = 0; i < A.size(); ++i)
            diag.push_back(A[i][i]);
        return diag;
    }

    std::pair<ArenaVector<double>, ArenaVector<ArenaVector<double>>> LinearSystemParser::ComputeEigenvalues(const ArenaVector<ArenaVector<double>> &inputA, int max_iterations) const noexcept
    {
        ArenaVector<ArenaVector<double>> A = inputA;
        int n = static_cast<int>(A.size());
        ArenaVector<ArenaVector<double>> EigenVectors = CreateIdentityMatrix(n);

        for (int k = 0; k < max_iterations; k++)
        {
            auto [Q, R] = GramSchmidt(A);
            if (Q.empty())
                break;
            A = MultiplyMatrices(R, Q);
            EigenVectors = MultiplyMatrices(EigenVectors, Q);
        }
        ArenaVector<double> eigenValues = GetDiagonal(A);
        return {eigenValues, EigenVectors};
    }

    bool LinearSystemParser::ParseLinearSystem(std::string_view input, ArenaVector<ArenaVector<double>> &A, ArenaVector<double> &b) const noexcept
    {
        std::string processed_input;
        processed_input.reserve(input.size());
        for (char c : input)
            if (!std::isspace(static_cast<unsigned char>(c)))
                processed_input += c;

        auto equations = Utils::Split(processed_input, ';');

        int N = equations.size();
        if (N == 0)
            return false;

        A.assign(N, ArenaVector<double>(N, 0.0));
        b.assign(N, 0.0);
        std::unordered_map<char, int> var_to_index;
        int var_count = 0;

        for (int i = 0; i < N; ++i)
        {
            std::string_view eq_sv(equations[i]);
            size_t equal_pos = eq_sv.find('=');
            if (equal_pos == std::string_view::npos) return false;

            std::string_view lhs = eq_sv.substr(0, equal_pos);
            std::string_view rhs = eq_sv.substr(equal_pos + 1);

            auto rhs_val = Utils::FastParseDouble(rhs);
            if (!rhs_val) return false;
            b[i] = *rhs_val;

            size_t pos = 0;
            while (pos < lhs.size())
            {
                char sign = '+';
                if (lhs[pos] == '+' || lhs[pos] == '-')
                {
                    sign = lhs[pos];
                    ++pos;
                }
                size_t start = pos;
                while (pos < lhs.size() && (std::isdigit(static_cast<unsigned char>(lhs[pos])) || lhs[pos] == '.' || std::isalpha(static_cast<unsigned char>(lhs[pos]))))
                    ++pos;

                std::string_view term = lhs.substr(start, pos - start);
                if (term.empty()) continue;

                double coefficient = 1.0;
                char variable = '\0';

                size_t var_pos = term.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

                if (var_pos != std::string_view::npos)
                {
                    if (var_pos > 0)
                    {
                        std::string_view coef_str = term.substr(0, var_pos);
                        auto c_val = Utils::FastParseDouble(coef_str);
                        if (!c_val) return false;
                        coefficient = *c_val;
                    }
                    variable = term[var_pos];
                }
                else
                {
                    auto t_val = Utils::FastParseDouble(term);
                    if (!t_val) return false;
                    b[i] -= (sign == '+' ? 1 : -1) * (*t_val);
                    continue;
                }

                if (sign == '-')
                    coefficient = -coefficient;

                auto [it, inserted] = var_to_index.try_emplace(variable, var_count);
                if (inserted) {
                    var_count++;
                }

                int col = it->second;
                if (col < N) {
                    A[i][col] += coefficient;
                }
            }
        }
        if (var_count != N)
            return false;
        return true;
    }

    double LinearSystemParser::DotProduct(const ArenaVector<double> &v1, const ArenaVector<double> &v2) const noexcept
    {
        if (v1.size() != v2.size())
            return 0.0;
        
        double sum = 0.0;
        size_t i = 0;
        const size_t size = v1.size();

        #ifdef AXIOM_SIMD_AVX2_ENABLED
        if (size >= 4) {
            __m256d vsum = _mm256_setzero_pd();
            for (; i + 3 < size; i += 4) {
                __m256d m1 = _mm256_loadu_pd(&v1[i]);
                __m256d m2 = _mm256_loadu_pd(&v2[i]);
                #ifdef AXIOM_SIMD_FMA_ENABLED
                    vsum = _mm256_fmadd_pd(m1, m2, vsum);
                #else
                    vsum = _mm256_add_pd(vsum, _mm256_mul_pd(m1, m2));
                #endif
            }
            // Horizontal sum
            alignas(32) double temp[4];
            _mm256_store_pd(temp, vsum);
            sum = temp[0] + temp[1] + temp[2] + temp[3];
        }
        #elif defined(__aarch64__)
        if (size >= 2) {
            float64x2_t vsum = vdupq_n_f64(0.0);
            for (; i + 1 < size; i += 2) {
                float64x2_t m1 = vld1q_f64(&v1[i]);
                float64x2_t m2 = vld1q_f64(&v2[i]);
                vsum = vfmaq_f64(vsum, m1, m2);
            }
            sum = vaddvq_f64(vsum);
        }
        #endif

        // Remainder
        for (; i < size; i++)
        {
            sum += v1[i] * v2[i];
        }
        return sum;
    }

    double LinearSystemParser::VectorNorm(const ArenaVector<double> &v) const noexcept { return std::sqrt(DotProduct(v, v)); }

    ArenaVector<double> LinearSystemParser::VectorScale(const ArenaVector<double> &v, const double scalar) const noexcept
    {
        ArenaVector<double> result = v;
        for (double &val : result)
            val *= scalar;
        return result;
    }

    ArenaVector<double> LinearSystemParser::VectorSub(const ArenaVector<double> &v1, const ArenaVector<double> &v2) const noexcept
    {
        if (v1.size() != v2.size())
            return {};
        ArenaVector<double> result = v1;
        for (size_t i = 0; i < result.size(); i++)
            result[i] -= v2[i];
        return result;
    }

    ArenaVector<ArenaVector<double>> LinearSystemParser::GetMinor(const ArenaVector<ArenaVector<double>> &A, int row, int col) const noexcept
    {
        int n = A.size();
        ArenaVector<ArenaVector<double>> minor;
        minor.reserve(n - 1);
        for (int i = 0; i < n; i++)
        {
            if (i == row)
                continue;
            ArenaVector<double> minor_row;
            minor_row.reserve(n - 1);
            for (int j = 0; j < n; j++)
            {
                if (j == col)
                    continue;
                minor_row.emplace_back(A[i][j]);
            }
            minor.emplace_back(std::move(minor_row));
        }
        return minor;
    }

    double LinearSystemParser::Determinant(const ArenaVector<ArenaVector<double>> &A) const noexcept
    {
        int n = A.size();
        if (n == 1)
            return A[0][0];

        ArenaVector<ArenaVector<double>> working_matrix = A;
        double det = 1.0;

        for (int i = 0; i < n; i++)
        {
            int max_row = i;
            for (int k = i + 1; k < n; k++)
            {
                if (std::abs(working_matrix[k][i]) > std::abs(working_matrix[max_row][i]))
                    max_row = k;
            }

            if (max_row != i)
            {
                std::swap(working_matrix[i], working_matrix[max_row]);
                det = -det;
            }

            if (std::abs(working_matrix[i][i]) < 1e-9)
                return 0.0;

            det *= working_matrix[i][i];

            for (int k = i + 1; k < n; k++)
            {
                double factor = working_matrix[k][i] / working_matrix[i][i];
                for (int j = i; j < n; j++)
                {
                    working_matrix[k][j] -= factor * working_matrix[i][j];
                }
            }
        }

        return std::abs(det) < 1e-9 ? 0.0 : det;
    }

    ArenaVector<ArenaVector<double>> LinearSystemParser::Transpose(const ArenaVector<ArenaVector<double>> &A) const noexcept
    {
        if (A.empty())
            return {};
        int N = A.size();
        int M = A[0].size();
        ArenaVector<ArenaVector<double>> T(M, ArenaVector<double>(N));
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < M; j++)
                T[j][i] = A[i][j];
        }
        return T;
    }

    std::pair<ArenaVector<ArenaVector<double>>, ArenaVector<ArenaVector<double>>> LinearSystemParser::GramSchmidt(const ArenaVector<ArenaVector<double>> &A) const noexcept
    {
        auto A_cols = Transpose(A);
        if (A_cols.empty())
            return {{}, {}};
        int N = A_cols.size();
        auto Q_cols = A_cols;
        ArenaVector<ArenaVector<double>> R(N, ArenaVector<double>(N, 0.0));
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < i; j++)
            {
                R[j][i] = DotProduct(Q_cols[j], A_cols[i]);
                auto projection = VectorScale(Q_cols[j], R[j][i]);
                Q_cols[i] = VectorSub(Q_cols[i], projection);
            }
            R[i][i] = VectorNorm(Q_cols[i]);
            if (std::abs(R[i][i]) > 1e-9)
            {
                Q_cols[i] = VectorScale(Q_cols[i], (1.0 / R[i][i]));
            }
            else
            {
                return {{}, {}};
            }
        }
        auto Q = Transpose(Q_cols);
        return {Q, R};
    }

    ArenaVector<double> LinearSystemParser::solve_linear_system(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<double> &b) const noexcept
    {
        int N = A.size();
        if (N == 0 || A[0].size() != N || b.size() != N)
            return {};
        ArenaVector<ArenaVector<double>> M(N, ArenaVector<double>(N + 1));
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; ++j)
                M[i][j] = A[i][j];
            M[i][N] = b[i];
        }
        for (int i = 0; i < N; i++)
        {
            int max_row = i;
            for (int k = i + 1; k < N; k++)
            {
                if (std::abs(M[k][i]) > std::abs(M[max_row][i]))
                    max_row = k;
            }
            std::swap(M[i], M[max_row]);
            if (std::abs(M[i][i]) < 1e-9)
                return {};
            for (int j = i + 1; j <= N; j++)
                M[i][j] /= M[i][i];
            for (int k = 0; k < N; k++)
            {
                if (k != i)
                {
                    double factor = M[k][i];
                    for (int j = i; j <= N; j++)
                        M[k][j] -= factor * M[i][j];
                }
            }
        }
        ArenaVector<double> solution(N);
        for (int i = 0; i < N; i++)
            solution[i] = M[i][N];
        return solution;
    }

    std::optional<ArenaVector<double>> LinearSystemParser::CramersRule(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<double> &b) const noexcept
    {
        int n = A.size();
        if (n != b.size())
            return std::nullopt;
        double detA = Determinant(A);
        if (std::abs(detA) < 1e-9)
            return std::nullopt;

        ArenaVector<double> solution(n);
        for (int i = 0; i < n; ++i)
        {
            auto Ai = A;
            for (int j = 0; j < n; ++j)
                Ai[j][i] = b[j];
            double detAi = Determinant(Ai);
            solution[i] = detAi / detA;
        }
        return solution;
    }

    ArenaVector<ArenaVector<double>> LinearSystemParser::ParseMatrixString(std::string_view input) const noexcept
    {
        ArenaVector<ArenaVector<double>> result;
        MatrixLexer lexer(input);
        ArenaVector<double> current_row;

        Token token = lexer.NextToken();

        if (token.type == TokenType::LBracket)
            token = lexer.NextToken();

        while (token.type != TokenType::End)
        {
            if (token.type == TokenType::LBracket)
            {
                if (!current_row.empty())
                {
                    result.emplace_back(current_row);
                    current_row.clear();
                }
            }
            else if (token.type == TokenType::Number)
            {
                auto val = Utils::FastParseDouble(token.value);
                if (val) {
                    current_row.emplace_back(*val);
                }
            }
            else if (token.type == TokenType::Semicolon || token.type == TokenType::RBracket)
            {
                if (!current_row.empty())
                {
                    result.emplace_back(std::move(current_row));
                    current_row.clear();
                }
            }

            token = lexer.NextToken();
        }

        if (!current_row.empty())
            result.emplace_back(std::move(current_row));
        if (result.empty())
            return {};

        size_t cols = result[0].size();
        for (const auto &row : result)
            if (row.size() != cols)
                return {};

        return result;
    }

} // namespace AXIOM





