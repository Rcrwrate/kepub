#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <thread>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <CLI/CLI.hpp>
#include <pugixml.hpp>

#include "html.h"
#include "http.h"
#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::esjzone;

namespace {

pugi::xml_document get_xml(const std::string &url, const std::string &proxy) {
  auto response = http_get(url, proxy);
  return kepub::html_to_xml(response);
}

std::pair<kepub::BookInfo, std::vector<kepub::Volume>> get_info(
    const std::string &book_id, bool translation, const std::string &proxy) {
  kepub::BookInfo book_info;

  const std::string url = "https://www.esjzone.cc/detail/" + book_id + ".html";
  klib::info("Download novel from {}", url);
  const auto doc = get_xml(url, proxy);

  // book name
  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                     "mb-3']/div[@class='col-md-9 book-detail']/h2")
                  .node();
  CHECK_NODE(node);
  book_info.name_ = kepub::trans_str(node.text().as_string(), translation);
  klib::info("Book name: {}", book_info.name_);

  // author
  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-9 book-detail']/ul")
             .node();
  if (!node.empty()) {
    std::string prefix = "作者:";
    for (const auto &child : node.children()) {
      if (child.child("strong").text().as_string() == prefix) {
        book_info.author_ =
            kepub::trans_str(child.child("a").text().as_string(), translation);
      }
    }
    klib::info("Author: {}", book_info.author_);
  }

  // introduction
  node =
      doc.select_node(
             "/html/body/div[@class='offcanvas-wrapper']/section/div/"
             "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='bg-secondary "
             "p-20 margin-top-1x']/div/div/div")
          .node();
  if (!node.empty()) {
    for (const auto &child : node.children()) {
      kepub::push_back(book_info.introduction_,
                       kepub::trans_str(child.text().as_string(), translation));
    }
  }

  // chapters
  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "padding-top-1x mb-3']/div/div/div[@class='tab-pane fade "
                "active show']/div[@id='chapterList']")
             .node();
  CHECK_NODE(node);

  std::vector<kepub::Volume> volumes;
  for (const auto &child : node.children("a")) {
    const std::string may_be_url = child.attribute("href").as_string();
    const auto title =
        kepub::trans_str(child.child("p").text().as_string(), translation);

    if (!(may_be_url.starts_with("https://www.esjzone.cc/") ||
          may_be_url.starts_with("https://www.esjzone.net/"))) {
      klib::warn("URL error: {}, title: {}", may_be_url, title);
    } else {
      if (std::empty(volumes)) {
        volumes.emplace_back();
      }

      volumes.back().chapters_.emplace_back(may_be_url, title);
    }
  }

  // cover
  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-3']/div[@class='product-gallery "
                "text-center mb-3']/a/img")
             .node();
  if (!node.empty()) {
    book_info.cover_path_ = node.attribute("src").as_string();
    klib::info("Cover url: {}", book_info.cover_path_);

    try {
      const auto image = http_get(book_info.cover_path_, proxy);
      const auto image_extension = kepub::image_to_extension(image);

      if (image_extension) {
        std::string cover_name = "cover" + *image_extension;
        klib::write_file(cover_name, true, image);
        klib::info("Cover downloaded successfully: {}", cover_name);
      } else {
        klib::warn("Image is not a supported format: {}",
                   book_info.cover_path_);
      }
    } catch (const klib::RuntimeError &err) {
      klib::warn("{}: {}", err.what(), book_info.cover_path_);
    }
  }

  return {book_info, volumes};
}

std::vector<std::string> get_content(const std::string &url, bool translation,
                                     const std::string &proxy) {
  auto doc = get_xml(url, proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 "
                     "p-r-30']/div[@class='forum-content mt-3']")
                  .node();
  CHECK_NODE(node);

  std::vector<std::string> result;

  const static std::string image_prefix = "[IMAGE] ";
  const static auto image_prefix_size = std::size(image_prefix);

  for (const auto &text : kepub::get_node_texts(node)) {
    for (const auto &line : klib::split_str(text, "\n")) {
      if (line.starts_with(image_prefix)) {
        try {
          const auto image_url = line.substr(image_prefix_size);
          const auto image = http_get(image_url, proxy);
          const auto image_extension = kepub::image_to_extension(image);
          if (!image_extension) {
            klib::warn("Image is not a supported format: {}", image_url);
            continue;
          }

          const auto image_stem = kepub::url_to_stem_name(image_url);
          auto new_image_name = image_stem + *image_extension;
          kepub::push_back(result, image_prefix + new_image_name);

          klib::write_file(new_image_name, true, image);
        } catch (const klib::RuntimeError &err) {
          klib::warn("{}: {}", err.what(), line);
        }
      } else {
        kepub::push_back(result, kepub::trans_str(line, translation));
      }
    }
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.footer(kepub::footer_str());
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  auto hardware_concurrency = std::thread::hardware_concurrency();
  std::int32_t max_concurrency = 0;
  app.add_option("-m,--multithreading", max_concurrency,
                 "Maximum number of concurrency to use when downloading")
      ->check(
          CLI::Range(1U, hardware_concurrency > 4 ? hardware_concurrency : 4))
      ->default_val(4);

  std::string proxy;
  app.add_flag("-p{http://127.0.0.1:1080},--proxy{http://127.0.0.1:1080}",
               proxy, "Use proxy")
      ->expected(0, 1);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);
  if (!std::empty(proxy)) {
    klib::info("Use proxy: {}", proxy);
  }

  klib::info("Maximum concurrency: {}", max_concurrency);
  if (max_concurrency > 4) {
    klib::warn("This maximum concurrency can be dangerous, please be careful");
  }

  klib::warn(
      "Volume division is not supported at the moment, please handle it "
      "manually");

  kepub::BookInfo book_info;
  std::vector<kepub::Volume> volumes;
  std::tie(book_info, volumes) = get_info(book_id, translation, proxy);

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
              chapter.texts_ = get_content(chapter.url_, translation, proxy);
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
