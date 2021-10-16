#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>
#include <klib/util.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>

#include "trans.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str(argv[0]));

  std::string file_name;
  app.add_option("file", file_name, "TXT file to be processed")->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  bool connect_chinese = false;
  app.add_flag("-c,--connect", connect_chinese,
               "Remove extra line breaks between Chinese");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);
  auto book_name =
      kepub::trans_str(std::filesystem::path(file_name).stem(), translation);

  std::vector<std::pair<std::string, std::vector<std::string>>> title_content;

  auto ref_vec = klib::read_file_line("ref.txt");
  auto size = std::size(ref_vec);

  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  for (std::size_t i = 0; i < size; ++i) {
    if (ref_vec[i].starts_with(title_prefix)) {
      auto title = ref_vec[i].substr(title_prefix_size);
      ++i;

      std::vector<std::string> content;
      for (; i < size && !ref_vec[i].starts_with(title_prefix); ++i) {
        content.push_back(ref_vec[i]);
      }
      --i;

      title_content.emplace_back(title, content);
    }
  }

  auto todo_vec = kepub::read_file_to_vec(file_name, translation);
  std::vector<std::string> result;
  for (const auto &line : todo_vec) {
    kepub::push_back(result, line, connect_chinese);
  }

  auto i = std::begin(result);
  for (const auto &[title, content] : title_content) {
    auto content_size = std::size(content);
    const auto compare_line_num = std::min(content_size, 4UL);

    std::vector<std::string> compare_vec;
    for (std::size_t index = 0; index < compare_line_num; ++index) {
      compare_vec.push_back(content[index]);
    }

    for (; i + compare_line_num - 1 < std::end(result); ++i) {
      std::vector<std::string> v;
      for (std::size_t index = 0; index < compare_line_num; ++index) {
        v.push_back(*(i + index));
      }

      if (std::equal(std::begin(compare_vec), std::end(compare_vec) - 1,
                     std::begin(v)) &&
          v.back().starts_with(compare_vec.back())) {
        break;
      }
    }

    if (i + compare_line_num - 1 == std::end(result)) {
      klib::error("error: {}", title);
    } else {
      i = result.insert(i, "\n[WEB] " + title + "\n");
    }
  }

  std::filesystem::rename(file_name, book_name + "-back-up.txt");

  auto &first_line = result.front();
  // first '\n'
  if (first_line.starts_with('\n')) {
    first_line.erase(std::begin(first_line));
  }

  auto &last_line = result.back();
  last_line.push_back('\n');
  klib::write_file(file_name, false, boost::join(result, "\n"));
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
