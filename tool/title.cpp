#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>
#include <klib/util.h>
#include <boost/algorithm/string.hpp>

#include "trans.h"
#include "util.h"

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);
  auto book_name = kepub::trans_str(std::filesystem::path(file_name).stem());

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

  auto todo_vec = kepub::read_file_to_vec(file_name);
  std::vector<std::string> result;
  for (const auto &line : todo_vec) {
    kepub::push_back(result, line, options.connect_chinese_);
  }

  auto i = std::begin(result);
  for (const auto &[title, content] : title_content) {
    auto ref_line1 = content[0];
    auto ref_line2 = content[1];
    auto ref_line3 = content[2];
    auto ref_line4 = content[3];

    for (; i + 3 < std::end(result); ++i) {
      if (ref_line1 == *i && ref_line2 == *(i + 1) && ref_line3 == *(i + 2) &&
          ref_line4 == *(i + 3)) {
        break;
      }
    }

    if (i + 3 == std::end(result)) {
      klib::error("error: {}", title);
    } else {
      i = result.insert(i, "\n[WEB] " + title + "\n");
    }
  }

  std::filesystem::rename(file_name, book_name + "-back-up.txt");

  auto &first_line = result.front();
  // first '\n'
  first_line.erase(std::begin(first_line));

  auto &last_line = result.back();
  last_line.push_back('\n');
  klib::write_file(file_name, false, boost::join(result, "\n"));
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}