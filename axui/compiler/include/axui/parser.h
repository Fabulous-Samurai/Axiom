#pragma once

#include "node.h"
#include <simdjson.h>
#include <string>
#include <vector>

namespace axui {

struct ParseError {
    std::string message;
    size_t position = 0;
};

class Parser {
public:
    UINode parse(const std::string& json_content);
    const std::vector<ParseError>& errors() const { return errors_; }
    bool hasErrors() const { return !errors_.empty(); }

private:
    std::vector<ParseError> errors_;

    UINode parseNode(simdjson::ondemand::object& obj);
    GlassParams parseGlass(simdjson::ondemand::object& obj);
    HoverParams parseHover(simdjson::ondemand::object& obj);
    LayoutParams parseLayout(simdjson::ondemand::object& obj);
    std::vector<Property> parseProperties(simdjson::ondemand::object& obj);
    Binding parseBinding(std::string_view expr);
};

} // namespace axui
