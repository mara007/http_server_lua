#include "utils.h"

#include <boost/log/trivial.hpp>

std::vector<std::string_view> tokenize(std::string_view data, std::string_view separator, bool consume_empty) {
    if (separator.empty())
        return {};

    size_t last_pos = 0;
    auto pos = data.find(separator);

    std::vector<std::string_view> result;
    while (pos != std::string::npos) {
        auto token = data.substr(last_pos, pos - last_pos);
        // BOOST_LOG_TRIVIAL(debug) << "token: " << token;

        if (not (consume_empty && token.empty()))
            result.push_back(token);

        last_pos = pos + separator.size();
        pos = data.find(separator, last_pos);
    }

    if (last_pos == data.size()) //skip last separator
        return result;

    auto token = data.substr(last_pos);
    // BOOST_LOG_TRIVIAL(debug) << "token: " << token;

    if (not (consume_empty && token.empty()))
        result.push_back(token);

    return result;
}

std::string to_lowercase(std::string_view s) {
    std::string result;
    std::transform(std::begin(s), std::end(s),
        std::back_inserter(result),
        [](auto c) {
            return std::tolower(c);
        }
    );

    return result;
}