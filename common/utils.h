#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <algorithm>


std::vector<std::string_view> tokenize(std::string_view data, std::string_view separator, bool consume_empty=false);

std::string to_lowercase(std::string_view s);