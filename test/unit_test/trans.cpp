#include <catch2/catch_test_macros.hpp>

#include "trans.h"

TEST_CASE("trans_str", "[trans]") {
  CHECK(kepub::trans_str("Ｑ０", false) == "Q0");
  CHECK(kepub::trans_str("“安装后”", false) == "“安装后”");
  CHECK(kepub::trans_str("&amp;", false) == "&");
  CHECK(kepub::trans_str("安裝後?", true) == "安装后？");
  CHECK(kepub::trans_str("，，，", false) == "，");
  CHECK(kepub::trans_str("安　装", false) == "安 装");
}
