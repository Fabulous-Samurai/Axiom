// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#include "plot_parser.h"

#include "fixed_vector.h"
#include "string_helpers.h"

namespace AXIOM {

EngineResult PlotParser::ParseAndExecute(std::string_view input) noexcept {
  const std::string_view prefix = "plot(";
  if (input.rfind(prefix, 0) != 0 || input.back() != ')') {
    return CreateErrorResult(CalcErr::ParseError);
  }

  const std::string_view args_content =
      input.substr(prefix.size(), input.size() - prefix.size() - 1);

  AXIOM::FixedVector<std::string_view, 16> args;
  size_t start = 0;
  int paren_depth = 0;
  for (size_t i = 0; i <= args_content.size(); ++i) {
    const char c = (i < args_content.size()) ? args_content[i] : ',';
    if (c == '(') {
      ++paren_depth;
    } else if (c == ')') {
      --paren_depth;
    } else if (c == ',' && paren_depth == 0) {
      const std::string_view arg =
          Utils::Trim(args_content.substr(start, i - start));
      if (!arg.empty()) {
        args.push_back(arg);
      }
      start = i + 1;
    }
  }

  if (args.size() != 5) {
    return CreateErrorResult(CalcErr::ArgumentMismatch);
  }

  AXIOM::PlotConfig cfg;
  auto xmin = Utils::FastParseDouble(args[1]);
  auto xmax = Utils::FastParseDouble(args[2]);
  auto ymin = Utils::FastParseDouble(args[3]);
  auto ymax = Utils::FastParseDouble(args[4]);

  if (!xmin || !xmax || !ymin || !ymax) {
    return CreateErrorResult(CalcErr::ParseError);
  }

  cfg.x_min = *xmin;
  cfg.x_max = *xmax;
  cfg.y_min = *ymin;
  cfg.y_max = *ymax;

  auto data = plot_engine_.ComputeFunctionData(std::string(args[0]), cfg);
  return CreateSuccessResult(std::move(data));
}

}  // namespace AXIOM
