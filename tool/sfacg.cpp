#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <klib/unicode.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <CLI/CLI.hpp>

#include "http.h"
#include "json.h"
#include "progress_bar.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::sfacg;

namespace {

bool show_user_info() {
  auto response = http_get("https://api.sfacg.com/user");
  const auto info = json_to_user_info(std::move(response));

  if (info.login_expired_) {
    return false;
  } else {
    klib::info("Use existing cookies, nick name: {}", info.nick_name_);
    return true;
  }
}

void login(const std::string &login_name, const std::string &password) {
  auto response = http_post("https://api.sfacg.com/sessions",
                            serialize(login_name, password));
  json_base(std::move(response));

  response = http_get("https://api.sfacg.com/user");
  const auto info = json_to_login_info(std::move(response));
  klib::info("Login successful, nick name: {}", info.user_info_.nick_name_);
}

kepub::BookInfo get_book_info(const std::string &book_id) {
  auto response = http_get("https://api.sfacg.com/novels/" + book_id,
                           {{"expand", "intro"}});
  auto info = json_to_book_info(std::move(response));

  klib::info("Book name: {}", info.name_);
  klib::info("Author: {}", info.author_);
  klib::info("Point: {}", info.point_);
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

std::vector<kepub::Volume> get_volume_chapter(const std::string &book_id) {
  klib::info("Start getting chapter information");

  auto response = http_get(fmt::format(
      FMT_COMPILE("https://api.sfacg.com/novels/{}/dirs"), book_id));

  return json_to_volumes(std::move(response));
}

std::optional<std::string> parse_image_url(const std::string &line) {
  const auto begin = line.find("https");
  if (begin == std::string::npos) {
    klib::warn("Invalid image URL: {}", line);
    return {};
  }

  const auto end = line.find("[/img]");
  if (end == std::string::npos) {
    klib::warn("Invalid image URL: {}", line);
    return {};
  }

  return line.substr(begin, end - begin);
}

std::vector<std::string> get_content(std::uint64_t chapter_id) {
  const auto id = std::to_string(chapter_id);
  auto response = http_get("https://api.sfacg.com/Chaps/" + id,
                           {{"chapsId", id}, {"expand", "content"}});

  const auto content_str = json_to_chapter_text(std::move(response));

  std::vector<std::string> content;
  for (auto &line : klib::split_str(content_str, "\n")) {
    klib::trim(line);

    if (line.starts_with("[img")) {
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

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);

  klib::info("Maximum concurrency: {}", max_concurrency);
  if (max_concurrency > 1) {
    klib::warn("This maximum concurrency can be dangerous, please be careful");
  }

  if (!show_user_info()) {
    const auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    login(login_name, password);
    klib::cleanse(password);
  }

  const auto book_info = get_book_info(book_id);
  auto volumes = get_volume_chapter(book_id);

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
              chapter.texts_ = get_content(chapter.chapter_id_);
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
