#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/url.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <CLI/CLI.hpp>
#include <gsl/assert>
#include <pugixml.hpp>

#include "html.h"
#include "http.h"
#include "json.h"
#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::masiro;

namespace {

pugi::xml_document get_xml(const std::string &url, const std::string &proxy) {
  auto response = http_get(url, proxy);
  return kepub::html_to_xml(response);
}

std::optional<std::string> get_user_info(const std::string &proxy) {
  auto doc = get_xml("https://masiro.me/admin/userCenterShow", proxy);
  auto node = doc.select_node("/html/head/title").node();
  CHECK_NODE(node);
  std::string title = node.text().as_string();

  if (title.find("登录") != std::string::npos) {
    return {};
  }

  node = doc.select_node(
                "/html/body/div/header/nav/div/ul/li[@class='dropdown user "
                "user-menu']/ul/li[@class='user-header']/p")
             .node();
  CHECK_NODE(node);

  return node.text().as_string();
}

bool show_user_info(const std::string &proxy) {
  auto nick_name = get_user_info(proxy);

  if (!nick_name) {
    return false;
  } else {
    klib::info("Use existing cookies, nick name: {}", *nick_name);
    return true;
  }
}

std::string get_token(const std::string &proxy) {
  auto doc = get_xml("https://masiro.me/admin/auth/login", proxy);
  auto node = doc.select_node(
                     "/html/body/div/section/div/div/div[@id='login']/form/"
                     "p[@class='login "
                     "button']/input[@name='_token']")
                  .node();
  CHECK_NODE(node);

  return node.attribute("value").as_string();
}

void login(const std::string &login_name, const std::string &password,
           const std::string &proxy) {
  auto response = http_post("https://masiro.me/admin/auth/login",
                            {{"username", login_name},
                             {"password", password},
                             {"remember", "1"},
                             {"_token", get_token(proxy)}},
                            {}, proxy);
  json_base(std::move(response));

  auto nick_name = get_user_info(proxy);
  if (!nick_name) {
    klib::error("Login failed");
  }

  klib::info("Login successful, nick name: {}", *nick_name);
}

std::string remove_image_url_quality(const std::string &image_url) {
  klib::URL url(image_url);

  auto query = url.query();
  query.erase("quality");
  url.set_query(query);

  return url.build();
}

std::tuple<std::string, kepub::BookInfo, std::vector<kepub::Volume>> get_info(
    const std::string &book_id, bool translation, const std::string &proxy) {
  kepub::BookInfo book_info;

  const std::string url =
      "https://masiro.me/admin/novelView?novel_id=" + book_id;
  klib::info("Download novel from {}", url);
  const auto doc = get_xml(url, proxy);

  auto node = doc.select_node("/html/head/meta[@name='csrf-token']").node();
  CHECK_NODE(node);
  std::string token = node.attribute("content").as_string();

  node = doc.select_node(
                "/html/body/div/div[@id='pjax-container']/div/"
                "section[@class='content']/div[1]/div/div/"
                "div[@class='box-body z-i']/div[@class='novel-title']")
             .node();
  CHECK_NODE(node);
  book_info.name_ = kepub::trans_str(node.text().as_string(), translation);

  node = doc.select_node(
                "/html/body/div/div[@id='pjax-container']/div/"
                "section[@class='content']/div[1]/div/div/"
                "div[@class='box-body "
                "z-i']/div[@class='n-detail']/div[@class='author']/a")
             .node();
  CHECK_NODE(node);
  book_info.author_ = kepub::trans_str(node.text().as_string(), translation);

  node = doc.select_node(
                "/html/body/div/div[@id='pjax-container']/div/"
                "section[@class='content']/div[1]/div/div/"
                "div[@class='box-footer z-i']/div")
             .node();
  CHECK_NODE(node);
  for (const auto &child : klib::split_str(node.text().as_string(), "\n")) {
    kepub::push_back(book_info.introduction_,
                     kepub::trans_str(child, translation));
  }

  node = doc.select_node(
                "/html/body/div/div[@id='pjax-container']/div/"
                "section[@class='content']/div[2]/div/div/"
                "div[@class='box-body']/div[@class='chapter-content']/ul")
             .node();
  CHECK_NODE(node);

  std::vector<kepub::Volume> volumes;
  for (const auto &child : node.children("li")) {
    if (child.attribute("class").value() == std::string("chapter-box")) {
      auto volume_name =
          kepub::trans_str(child.child("b").text().as_string(), translation);
      volumes.emplace_back(volume_name);
    } else {
      if (std::empty(volumes)) {
        volumes.emplace_back();
      }

      auto chapters = child.child("ul");
      for (const auto &chapter : chapters.children()) {
        if (chapter.name() == std::string("a")) {
          std::string chapter_url = std::string("https://masiro.me") +
                                    chapter.attribute("href").as_string();
          volumes.back().chapters_.emplace_back();
          volumes.back().chapters_.back().url_ = chapter_url;
        } else if (chapter.name() == std::string("li")) {
          auto chapter_title = kepub::trans_str(
              chapter.child("span").text().as_string(), translation);
          volumes.back().chapters_.back().title_ = chapter_title;

          std::string pay_str = kepub::trans_str(
              chapter.child("small").text().as_string(), translation);
          if (!std::empty(pay_str) && pay_str != "已购") {
            Expects(pay_str.ends_with("G"));
            volumes.back().chapters_.back().pay_ = std::stoi(pay_str);
          }
        }
      }
    }
  }

  node = doc.select_node(
                "/html/body/div/div[@id='pjax-container']/div/"
                "section[@class='content']/div[1]/div/div/div[@class='box-body "
                "z-i']/div[@class='with-border']/div/span/a/img")
             .node();
  CHECK_NODE(node);
  book_info.cover_path_ = remove_image_url_quality(
      std::string("https://masiro.me") + node.attribute("src").as_string());

  klib::info("Book name: {}", book_info.name_);
  klib::info("Author: {}", book_info.author_);
  klib::info("Cover url: {}", book_info.cover_path_);

  try {
    const auto image = http_get(book_info.cover_path_, proxy);
    const auto image_extension = kepub::image_to_extension(image);

    if (image_extension) {
      std::string cover_name = "cover" + *image_extension;
      klib::write_file(cover_name, true, image);
      klib::info("Cover downloaded successfully: {}", cover_name);
    } else {
      klib::warn("Image is not a supported format: {}", book_info.cover_path_);
    }
  } catch (const klib::RuntimeError &err) {
    klib::warn("{}: {}", err.what(), book_info.cover_path_);
  }

  return {token, book_info, volumes};
}

std::string get_chapter_id(const std::string &chapter_url) {
  auto map = klib::URL(chapter_url).query();
  return map.at("cid");
}

void pay(const std::string &chapter_url, std::int32_t chapter_pay,
         const std::string &token, const std::string &proxy) {
  auto response = http_post(
      "https://masiro.me/admin/pay",
      {{"type", "2"},
       {"object_id", get_chapter_id(chapter_url)},
       {"cost", std::to_string(chapter_pay)}},
      {{"X-Requested-With", "XMLHttpRequest"}, {"X-CSRF-Token", token}}, proxy);
  json_base(std::move(response));
}

std::vector<std::string> get_content(const std::string &url, bool translation,
                                     const std::string &proxy) {
  auto doc = get_xml(url, proxy);

  auto node = doc.select_node(
                     "/html/body/div/div[@id='pjax-container']/div/"
                     "section[@class='content']/div[1]/div/div/"
                     "div[@class='box-body nvl-content']")
                  .node();
  CHECK_NODE(node);

  std::vector<std::string> result;

  const static std::string image_prefix = "[IMAGE] ";
  const static auto image_prefix_size = std::size(image_prefix);

  for (const auto &text : kepub::get_node_texts(node)) {
    for (const auto &line : klib::split_str(text, "\n")) {
      if (line.starts_with(image_prefix)) {
        try {
          const auto image_url =
              remove_image_url_quality(line.substr(image_prefix_size));
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
      ->default_val(1);

  std::string proxy;
  app.add_flag("-p{http://127.0.0.1:1080},--proxy{http://127.0.0.1:1080}",
               proxy, "Use proxy")
      ->expected(0, 1);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);

  klib::info("Maximum concurrency: {}", max_concurrency);
  if (max_concurrency > 1) {
    klib::warn("This maximum concurrency can be dangerous, please be careful");
  }

  if (!show_user_info(proxy)) {
    const auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    login(login_name, password, proxy);
    klib::cleanse(password);
  }

  std::string token;
  kepub::BookInfo book_info;
  std::vector<kepub::Volume> volumes;
  std::tie(token, book_info, volumes) = get_info(book_id, translation, proxy);

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

              if (chapter.pay_ > 0) {
                pay(chapter.url_, chapter.pay_, token, proxy);
              }
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
