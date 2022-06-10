#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/unicode.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <CLI/CLI.hpp>
#include <pugixml.hpp>

#include "aes.h"
#include "geetest.h"
#include "http.h"
#include "json.h"
#include "novel.h"
#include "progress_bar.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::ciweimao;

namespace {

const auto token_path = klib::get_env("HOME").value_or("/tmp") + "/.ciweimao";

bool show_user_info(const Token &token, bool new_version) {
  auto response = http_post("https://app.hbooker.com/reader/get_my_info",
                            {{"account", token.account_},
                             {"reader_id", token.reader_id_},
                             {"login_token", token.login_token_}},
                            new_version);
  const auto info = json_to_user_info(kepub::decrypt_no_iv(response));

  if (info.login_expired_) {
    return false;
  } else {
    klib::info("Use existing login token, nick name: {}", info.nick_name_);
    return true;
  }
}

std::optional<Token> try_read_token(bool new_version) {
  if (!std::filesystem::exists(token_path)) {
    klib::warn("Login required to access this resource");
    return {};
  }

  const auto json = klib::read_file(token_path, false);

  Token token;
  try {
    token = json_to_token(kepub::decrypt(json));
  } catch (...) {
    klib::warn("Failed to read local user information, please enter again");
    return {};
  }

  if (show_user_info(token, new_version)) {
    return token;
  } else {
    return {};
  }
}

void write_token(const Token &token) {
  klib::write_file(token_path, true,
                   kepub::encrypt(serialize(token.login_token_,
                                            token.reader_id_, token.account_)));
}

bool use_geetest(bool new_version) {
  auto response =
      http_post("https://app.hbooker.com/signup/use_geetest", {}, new_version);
  return json_to_use_geetest(kepub::decrypt_no_iv(response));
}

LoginInfo login(const std::string &login_name, const std::string &password,
                bool new_version) {
  std::string response;

  if (new_version && use_geetest(new_version)) {
    klib::info("Captcha required");

    auto [challenge, validate] = get_geetest_validate(login_name);
    response = http_post("https://app.hbooker.com/signup/login",
                         {{"login_name", login_name},
                          {"passwd", password},
                          {"geetest_seccode", validate + "|jordan"},
                          {"geetest_validate", validate},
                          {"geetest_challenge", challenge}},
                         new_version);
  } else {
    response = http_post("https://app.hbooker.com/signup/login",
                         {{"login_name", login_name}, {"passwd", password}},
                         new_version);
  }

  auto info = json_to_login_info(kepub::decrypt_no_iv(response));

  klib::info("Login successful, nick name: {}", info.user_info_.nick_name_);
  return info;
}

kepub::BookInfo get_book_info(const Token &token, const std::string &book_id,
                              bool new_version) {
  auto response = http_post("https://app.hbooker.com/book/get_info_by_id",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"book_id", book_id}},
                            new_version);
  auto info = json_to_book_info(kepub::decrypt_no_iv(response));

  klib::info("Book name: {}", info.name_);
  klib::info("Author: {}", info.author_);
  klib::info("Cover url: {}", info.cover_path_);

  try {
    const auto image = http_get_rss(info.cover_path_);
    const auto image_extension = kepub::image_to_extension(image);

    if (image_extension) {
      std::string cover_name = "cover" + *image_extension;
      klib::write_file(cover_name, true, image);
      klib::info("Cover downloaded successfully: {}", cover_name);
    } else {
      klib::warn("Image is not a supported format: {}", info.cover_path_);
    }
  } catch (const klib::RuntimeError &err) {
    klib::warn("{}: {}", err.what(), info.cover_path_);
  }

  return info;
}

std::vector<kepub::Volume> get_volume_chapter(const Token &token,
                                              const std::string &book_id,
                                              bool new_version) {
  klib::info("Start getting chapter information");

  auto response = http_post(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_new",
      {{"account", token.account_},
       {"login_token", token.login_token_},
       {"book_id", book_id}},
      new_version);

  return json_to_volumes(kepub::decrypt_no_iv(response));
}

std::string get_chapter_command(const Token &token,
                                const std::string &chapter_id,
                                bool new_version) {
  auto response = http_post("https://app.hbooker.com/chapter/get_chapter_cmd",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"chapter_id", chapter_id}},
                            new_version);

  return json_to_chapter_command(kepub::decrypt_no_iv(response));
}

std::optional<std::string> parse_image_url(const std::string &line) {
  pugi::xml_document doc;
  doc.load_string(line.c_str());
  std::string image_url = doc.child("img").attribute("src").as_string();

  if (std::empty(image_url)) {
    klib::warn("Invalid image URL: {}", line);
    return {};
  }

  return image_url;
}

std::vector<std::string> get_content(const Token &token,
                                     std::uint64_t chapter_id,
                                     bool new_version) {
  const auto id = std::to_string(chapter_id);
  const auto chapter_command = get_chapter_command(token, id, new_version);
  auto response = http_post("https://app.hbooker.com/chapter/get_cpt_ifm",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"chapter_id", id},
                             {"chapter_command", chapter_command}},
                            new_version);
  const auto encrypt_content_str =
      json_to_chapter_text(kepub::decrypt_no_iv(response));
  const auto content_str =
      kepub::decrypt_no_iv(encrypt_content_str, chapter_command);

  std::vector<std::string> content;
  for (auto &line : klib::split_str(content_str, "\n")) {
    klib::trim(line);

    if (line.starts_with("<img src")) {
      const auto image_url = parse_image_url(line);
      if (!image_url) {
        continue;
      }

      try {
        const auto image_stem = kepub::url_to_stem_name(*image_url);
        const auto image = http_get_rss(*image_url);
        const auto image_extension = kepub::image_to_extension(image);
        if (!image_extension) {
          klib::warn("Image is not a supported format: {}", *image_url);
          continue;
        }

        const auto image_name = image_stem + *image_extension;
        klib::write_file(image_name, true, image);
        line = "[IMAGE] " + image_name;
      } catch (const klib::RuntimeError &err) {
        klib::warn("{}: {}", err.what(), line);
        continue;
      }
    }

    kepub::push_back(content, line);
  }

  return content;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.footer(kepub::footer_str());
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  auto hardware_concurrency = std::thread::hardware_concurrency();
  std::int32_t max_concurrency = 0;
  app.add_option("-m,--multithreading", max_concurrency,
                 "Maximum number of concurrency to use when downloading")
      ->check(
          CLI::Range(1U, hardware_concurrency > 4 ? hardware_concurrency : 4))
      ->default_val(1);

  // TODO Delete when old version is not available
  bool new_version = false;
  app.add_flag("-n,--new-version", new_version,
               "With newer versions, you may need to open your browser and "
               "pass the captcha");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);

  klib::info("Maximum concurrency: {}", max_concurrency);
  if (max_concurrency > 4) {
    klib::warn("This maximum concurrency can be dangerous, please be careful");
  }

  Token token;
  if (auto may_token = try_read_token(new_version); may_token.has_value()) {
    token = *may_token;
  } else {
    auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    token = login(login_name, password, new_version).token_;
    klib::cleanse(password);
    write_token(token);
  }

  auto book_info = get_book_info(token, book_id, new_version);
  auto volumes = get_volume_chapter(token, book_id, new_version);

  std::size_t chapter_count = 0;
  for (const auto &volume : volumes) {
    chapter_count += std::size(volume.chapters_);
  }

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(chapter_count, book_info.name_);

  oneapi::tbb::task_arena limited(max_concurrency);
  oneapi::tbb::task_group task_group;

  for (auto &volume : volumes) {
    limited.execute([&] {
      task_group.run([&] {
        oneapi::tbb::parallel_for_each(
            volume.chapters_, [&](kepub::Chapter &chapter) {
              bar.set_postfix_text(chapter.title_);
              bar.tick();
              chapter.texts_ =
                  get_content(token, chapter.chapter_id_, new_version);
            });
      });
    });
    limited.execute([&] { task_group.wait(); });
  }

  kepub::generate_txt(book_info, volumes);
  klib::info("Novel '{}' download completed", book_info.name_);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
