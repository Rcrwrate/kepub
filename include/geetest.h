#pragma once

#include <string>
#include <utility>

#include "kepub_export.h"

namespace kepub::ciweimao {

std::pair<std::string, std::string> KEPUB_EXPORT
get_geetest_validate(const std::string &user_id);

}  // namespace kepub::ciweimao
