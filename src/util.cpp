#include "util.h"

#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <klib/log.h>
#include <klib/mime.h>
#include <klib/qr_code.h>
#include <klib/unicode.h>
#include <klib/url.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <parallel_hashmap/phmap.h>
#include <re2/re2.h>
#include <gsl/assert>

#include "trans.h"

namespace kepub {

namespace {

bool start_with_chinese(const std::string &str) {
  return klib::is_cjk(klib::first_code_point(str));
}

bool end_with_chinese(const std::string &str) {
  return klib::is_cjk(klib::last_code_point(str));
}

bool is_punctuation(char32_t code_point) {
  return code_point == U'◇' || klib::is_chinese_punctuation(code_point);
}

bool end_with_punctuation(const std::string &str) {
  return klib::is_chinese_punctuation(klib::last_code_point(str));
}

}  // namespace

std::string footer_str() {
  std::string result;

  result.append(
      "Report bugs to https://github.com/KaiserLancelot/kepub/issues\n\n");
  result.append(
      "You can use the link or QR code to contact me via Telegram\n\n");
  result.append("https://t.me/kaiserlancelot\n\n");
  result.append(klib::qr_code_to_utf8("https://t.me/kaiserlancelot", 2));

  return result;
}

void check_file_exist(const std::string &file_name) {
  if (!std::filesystem::is_regular_file(file_name)) {
    klib::error("The file not exist: '{}'", file_name);
  }
}

void check_is_txt_file(const std::string &file_name) {
  check_file_exist(file_name);

  if (std::filesystem::path(file_name).extension() != ".txt") {
    klib::error("Need a txt file: {}", file_name);
  }
}

void check_is_epub_file(const std::string &file_name) {
  check_file_exist(file_name);

  if (std::filesystem::path(file_name).extension() != ".epub") {
    klib::error("Need a epub file: {}", file_name);
  }
}

void remove_file_or_dir(const std::string &path) {
  if (!std::filesystem::exists(path)) {
    klib::error("The item does not exist: {}", path);
  }

  if (std::filesystem::is_regular_file(path)) {
    if (!std::filesystem::remove(path)) {
      klib::error("File deletion failed: {}", path);
    }
  } else if (std::filesystem::is_directory(path)) {
    if (std::filesystem::remove_all(path) == 0) {
      klib::error("Folder deletion failed: {}", path);
    }
  } else {
    klib::error("Not a file or folder");
  }
}

void check_is_book_id(const std::string &book_id) {
  if (!std::all_of(std::begin(book_id), std::end(book_id),
                   [](char c) { return std::isdigit(c); })) {
    klib::error("Need a book id: {}", book_id);
  }
}

std::vector<std::string> read_file_to_vec(const std::string &file_name,
                                          bool translation) {
  std::ifstream ifs(file_name);
  if (!ifs) {
    klib::error("Failed to open file: '{}'", file_name);
  }

  std::vector<std::string> result;
  result.reserve(16384);

  std::string line;
  while (std::getline(ifs, line)) {
    result.push_back(line);
  }

  oneapi::tbb::parallel_for_each(result, [&](std::string &str) {
    str = trans_str(str, translation);
    if (!klib::validate_utf8(str)) {
      klib::error("Invalid UTF-8: {}", str);
    }
  });

  std::erase_if(result,
                [](const std::string &line) { return std::empty(line); });

  return result;
}

void str_check(const std::string &str) {
  static phmap::flat_hash_set<char32_t> set;

  auto copy = str;
  std::erase_if(copy, [](char c) { return std::isalnum(c) || c == ' '; });

  for (auto c : klib::utf8_to_utf32(copy)) {
    if (!klib::is_cjk(c) && !is_punctuation(c)) {
      if (set.contains(c)) {
        continue;
      }

      set.insert(c);

      klib::warn("Unknown character: {} in {}",
                 klib::utf32_to_utf8(std::u32string(&c, 1)), str);
    }
  }
}

std::int32_t str_size(const std::string &str) {
  std::int32_t count = 0;

  for (auto c : klib::utf8_to_utf32(str)) {
    if (klib::is_cjk(c)) {
      ++count;
    }
  }

  return count;
}

void volume_name_check(const std::string &volume_name) {
  static re2::RE2 regex = R"(第([一二三四五六七八九十]|[0-9]){1,3}卷 .+)";

  if (!re2::RE2::FullMatch(volume_name, regex)) {
    klib::warn("Irregular volume name format: {}", volume_name);
    return;
  }

  str_check(volume_name);
}

void title_check(const std::string &title) {
  static re2::RE2 regex =
      R"(第([零一二三四五六七八九十百千]|[0-9]){1,7}[章话] .+)";

  if (!re2::RE2::FullMatch(title, regex)) {
    klib::warn("Irregular title format: {}", title);
    return;
  }

  str_check(title);
}

void push_back(std::vector<std::string> &texts, const std::string &str,
               bool connect, bool check) {
  if (std::empty(str)) {
    return;
  }

  if (std::empty(texts)) {
    texts.push_back(str);
    return;
  }

  if (texts.back().ends_with("，")) {
    if ((start_with_chinese(str) || std::isalnum(str.front())) ||
        (str.starts_with("—") || str.starts_with("“") ||
         str.starts_with("「") || str.starts_with("『") ||
         str.starts_with("《") || str.starts_with("[") ||
         str.starts_with("【") || str.starts_with("（"))) {
      texts.back().append(str);
    } else {
      if (check) {
        klib::warn("Punctuation may be wrong: {}, previous row: {}", str,
                   texts.back());
      }
      texts.push_back(str);
    }
  } else if (str.starts_with("！") || str.starts_with("？") ||
             str.starts_with("，") || str.starts_with("。") ||
             str.starts_with("、") || str.starts_with("”") ||
             str.starts_with("」") || str.starts_with("』") ||
             str.starts_with("》") || str.starts_with("]") ||
             str.starts_with("】") || str.starts_with("）")) {
    if (!end_with_punctuation(texts.back())) {
      texts.back().append(str);
    } else {
      if (check) {
        if (!(str.starts_with("！") || str.starts_with("？"))) {
          klib::warn("Punctuation may be wrong: {}", str);
        }
      }
      texts.push_back(str);
    }
  } else if (connect && std::isalpha(texts.back().back()) &&
             std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else if (connect && end_with_chinese(texts.back()) &&
             std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else if (connect && std::isalpha(texts.back().back()) &&
             start_with_chinese(str)) {
    texts.back().append(" " + str);
  } else if (connect && end_with_chinese(texts.back()) &&
             start_with_chinese(str)) {
    texts.back().append(str);
  } else {
    texts.push_back(str);
  }
}

void push_back(std::vector<std::string> &texts, const std::string &str) {
  if (auto std_str = klib::trim_copy(str); !std::empty(std_str)) {
    texts.push_back(std::move(std_str));
  }
}

std::string get_login_name() {
  std::string login_name;

  std::cout << "login name: ";
  std::cin >> login_name;
  if (std::empty(login_name)) {
    klib::error("login name is empty");
  }

  return login_name;
}

std::string get_password() {
  std::string password = getpass("password: ");

  if (std::empty(password)) {
    klib::error("password is empty");
  }

  return password;
}

std::string num_to_str(std::int32_t i) {
  Expects(i >= 0);

  auto str = std::to_string(i);
  if (i < 10) {
    return "00" + str;
  } else if (i < 100) {
    return "0" + str;
  } else {
    return str;
  }
}

std::string url_to_stem_name(const std::string &str) {
  klib::URL url(str);
  return std::filesystem::path(url.path()).stem().string();
}

std::optional<std::string> check_is_supported_format(
    const std::string &file_name) {
  if (file_name.ends_with(".jpg") || file_name.ends_with(".jpeg")) {
    return ".jpg";
  } else if (file_name.ends_with(".png")) {
    return ".png";
  } else if (file_name.ends_with(".webp")) {
    return ".webp";
  } else {
    klib::warn("Image is not a supported format : {}", file_name);
    return {};
  }
}

std::optional<std::string> image_to_extension(const std::string &image) {
  if (klib::is_jpeg(image)) {
    return ".jpg";
  } else if (klib::is_png(image)) {
    return ".png";
  } else if (klib::is_webp(image)) {
    return ".webp";
  } else {
    return {};
  }
}

std::string stem(const std::string &path) {
  return std::filesystem::path(path).stem().string();
}

std::string make_book_name_legal(const std::string &file_name) {
  auto new_file_name = klib::make_file_name_legal(file_name);
  if (new_file_name != file_name) {
    klib::warn(
        "The title of the book is illegal, and the title of the book has been "
        "changed from '{}' to '{}'",
        file_name, new_file_name);
  }

  return new_file_name;
}

void generate_txt(const BookInfo &book_info,
                  const std::vector<Volume> &volumes) {
  std::ostringstream oss;

  oss << "[AUTHOR]"
      << "\n\n";
  oss << book_info.author_ << "\n\n";

  oss << "[INTRO]"
      << "\n\n";
  for (const auto &line : book_info.introduction_) {
    oss << line << "\n";
  }
  oss << "\n";

  // old_name new_name
  phmap::flat_hash_map<std::string, std::string> image_name_map;
  for (const auto &volume : volumes) {
    if (std::empty(volume.chapters_)) {
      continue;
    }

    if (!std::empty(volume.title_)) {
      oss << "[VOLUME] " << volume.title_ << "\n\n";
    }

    for (const auto &chapter : volume.chapters_) {
      oss << "[WEB] " << chapter.title_ << "\n\n";

      for (const auto &line : chapter.texts_) {
        static std::int32_t image_count = 1;
        const static std::string image_prefix = "[IMAGE] ";
        const static auto image_prefix_size = std::size(image_prefix);

        if (line.starts_with(image_prefix)) [[unlikely]] {
          auto image_name = line.substr(image_prefix_size);

          if (auto iter = image_name_map.find(image_name);
              iter != std::end(image_name_map)) {
            oss << image_prefix << iter->second << '\n';
          } else {
            if (!std::filesystem::exists(image_name)) {
              klib::warn("Image not exists: {}", image_name);
              continue;
            }

            auto ext = check_is_supported_format(image_name);

            if (ext) {
              auto new_image_name = num_to_str(image_count++) + *ext;
              oss << image_prefix << new_image_name << '\n';
              std::filesystem::rename(image_name, new_image_name);

              image_name_map.emplace(image_name, new_image_name);
            }
          }
        } else [[likely]] {
          oss << line << '\n';
        }
      }
      oss << '\n';
    }
  }

  std::string str = oss.str();
  // '\n'
  str.pop_back();

  std::ofstream book_ofs(make_book_name_legal(book_info.name_) + ".txt");
  book_ofs << str << std::flush;
}

}  // namespace kepub
