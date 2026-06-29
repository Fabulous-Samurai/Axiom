#pragma once

#include "node.h"
#include <simdjson.h>
#include <string>
#include <vector>

namespace AXIOM { template<typename T> class ArenaAllocator; }

namespace axui {

struct ParseError {
    std::string message;
    size_t position = 0;
};

class Parser {
public:
    UINode* parse(const std::string& json_content, AXIOM::ArenaAllocator<UINode>& arena);
    const std::vector<ParseError>& errors() const { return errors_; }
    bool hasErrors() const { return !errors_.empty(); }

private:
    std::vector<ParseError> errors_;

    void parseNodeInto(simdjson::ondemand::object& obj, UINode& node, AXIOM::ArenaAllocator<UINode>& arena);
    GlassParams parseGlass(simdjson::ondemand::object& obj);
    HoverParams parseHover(simdjson::ondemand::object& obj);
    LayoutParams parseLayout(simdjson::ondemand::object& obj);
    void parsePropertiesInto(simdjson::ondemand::object& obj, UINode& node, AXIOM::ArenaAllocator<UINode>& arena);
    Binding parseBinding(std::string_view expr);
};

} // namespace axui
