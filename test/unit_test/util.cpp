#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "util.h"

TEST_CASE("check_is_book_id", "[util]") {
  REQUIRE_NOTHROW(kepub::check_is_book_id("12345"));
}

TEST_CASE("title_check", "[util]") {
  REQUIRE_NOTHROW(kepub::title_check("第一章 被俘虏的开始"));
  REQUIRE_NOTHROW(kepub::title_check("第一百三十二章 标标标标标标标标标"));
  REQUIRE_NOTHROW(kepub::title_check("第123章 标题标标标标"));

  REQUIRE_NOTHROW(kepub::title_check("第一章 "));
  REQUIRE_NOTHROW(kepub::title_check("第1二3话"));
  REQUIRE_NOTHROW(kepub::title_check("第123话标题"));
  REQUIRE_NOTHROW(kepub::title_check("123话 标题"));
}

TEST_CASE("volume_name_check", "[util]") {
  REQUIRE_NOTHROW(kepub::volume_name_check("第三十二卷 标标标标标标标标标"));
  REQUIRE_NOTHROW(kepub::volume_name_check("第123话 标题标标标标"));
  REQUIRE_NOTHROW(kepub::volume_name_check("第1卷 "));
}

TEST_CASE("push_back", "[util]") {
  std::vector<std::string> texts;
  std::string str = "第1卷\u000a";
  kepub::push_back(texts, str);

  REQUIRE(texts.front() == "第1卷");
}
