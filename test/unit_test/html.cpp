#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <pugixml.hpp>

#include "html.h"

TEST_CASE("HTML", "[html]") {
  std::vector<std::string> htmls{
      R"(<div>「唔嗯。作為<ruby>不死生物<rp>(</rp><rt>Undead</rt><rp>)</rp></ruby>，愉悅的場面便是無上的獎勵。就讓吾將所有敵人化作刀下亡魂」</div>)",
      R"(<br>男子名叫佛利歐，現在是從小孩到大人都具有高人氣的卡片遊戲「亞斯特利亞傳奇」的發行商，「<ruby><rb>魔書公司</rb><rp>(</rp><rt>Grimoire Company</rt><rp>)</rp></ruby>」的業務。</br>)"};

  std::vector<std::string> results{
      R"(「唔嗯。作為不死生物(Undead)，愉悅的場面便是無上的獎勵。就讓吾將所有敵人化作刀下亡魂」)",
      R"(男子名叫佛利歐，現在是從小孩到大人都具有高人氣的卡片遊戲「亞斯特利亞傳奇」的發行商，「魔書公司(Grimoire Company)」的業務。)"};

  std::size_t index = 0;
  for (const auto &html : htmls) {
    pugi::xml_document doc;
    doc.load_string(html.c_str());
    auto result = kepub::get_node_texts(doc);
    CHECK(result == std::vector<std::string>{results[index++]});
  }
}
