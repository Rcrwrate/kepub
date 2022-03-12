#include <exception>
#include <string>
#include <tuple>
#include <vector>

#include <klib/exception.h>
#include <klib/html.h>
#include <klib/log.h>
#include <klib/util.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "http.h"
#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

namespace {

void do_get_node_texts(const pugi::xml_node &node, std::string &str) {
  if (node.children().begin() == node.children().end()) {
    str += node.text().as_string();
  } else {
    for (const auto &child : node.children()) {
      if (node.name() == std::string("p") || node.name() == std::string("br")) {
        str += "\n";
      }
      do_get_node_texts(child, str);
    }
  }
}

std::vector<std::string> get_node_texts(const pugi::xml_node &node) {
  std::vector<std::string> result;

  for (const auto &child : node.children()) {
    std::string str;
    do_get_node_texts(child, str);
    result.push_back(str);
  }

  return result;
}

pugi::xml_document get_xml(const std::string &url, const std::string &proxy) {
  auto response = http_get(url, proxy);

  auto xml = klib::html_tidy(response.text(), true);
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  return doc;
}

std::pair<kepub::BookInfo, std::vector<std::pair<std::string, std::string>>>
get_info(const std::string &book_id, bool translation,
         const std::string &proxy) {
  kepub::BookInfo book_info;

  auto doc =
      get_xml("https://www.esjzone.cc/detail/" + book_id + ".html", proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                     "mb-3']/div[@class='col-md-9 book-detail']/h2")
                  .node();
  book_info.name_ = kepub::trans_str(node.text().as_string(), translation);

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-9 book-detail']/ul")
             .node();

  std::string prefix = "作者:";
  for (const auto &child : node.children()) {
    if (child.child("strong").text().as_string() == prefix) {
      book_info.author_ =
          kepub::trans_str(child.child("a").text().as_string(), translation);
    }
  }

  node =
      doc.select_node(
             "/html/body/div[@class='offcanvas-wrapper']/section/div/"
             "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='bg-secondary "
             "p-20 margin-top-1x']/div/div/div")
          .node();

  for (const auto &child : node.children()) {
    kepub::push_back(book_info.introduction_,
                     kepub::trans_str(child.text().as_string(), translation));
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "padding-top-1x mb-3']/div/div/div[@class='tab-pane fade "
                "active show']/div[@id='chapterList']")
             .node();

  std::vector<std::pair<std::string, std::string>> titles_and_urls;
  for (const auto &child : node.children("a")) {
    auto title =
        kepub::trans_str(child.child("p").text().as_string(), translation);

    std::string may_be_url = child.attribute("href").as_string();
    if (!may_be_url.starts_with("https://www.esjzone.cc/")) {
      klib::warn("url error: {}, title: {}", may_be_url, title);
    } else {
      titles_and_urls.emplace_back(title, may_be_url);
    }
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-3']/div[@class='product-gallery "
                "text-center mb-3']/a/img")
             .node();
  book_info.cover_path_ = node.attribute("src").as_string();

  klib::info("Book name: {}", book_info.name_);
  klib::info("Author: {}", book_info.author_);
  klib::info("Cover url: {}", book_info.cover_path_);

  std::string cover_name = "cover.jpg";
  auto response = http_get(book_info.cover_path_, proxy);
  response.save_to_file(cover_name);
  klib::info("Cover downloaded successfully: {}", cover_name);

  return {book_info, titles_and_urls};
}

std::vector<std::string> get_content(const std::string &url, bool translation,
                                     const std::string &proxy) {
  auto doc = get_xml(url, proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 "
                     "p-r-30']/div[@class='forum-content mt-3']")
                  .node();

  std::vector<std::string> result;
  for (const auto &text : get_node_texts(node)) {
    for (const auto &line : klib::split_str(text, "\n")) {
      kepub::push_back(result, kepub::trans_str(line, translation));
    }
  }

  if (std::empty(result)) {
    klib::warn("No text: {}", url);
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  std::string proxy;
  app.add_flag("-p{http://127.0.0.1:1080},--proxy{http://127.0.0.1:1080}",
               proxy, "Use proxy")
      ->expected(0, 1);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);
  if (!std::empty(proxy)) {
    klib::info("Use proxy: {}", proxy);
  }

  auto [book_info, titles_and_urls] = get_info(book_id, translation, proxy);

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(book_info.name_, std::size(titles_and_urls));
  std::vector<kepub::Chapter> chapters;
  for (const auto &[title, urls] : titles_and_urls) {
    bar.set_postfix_text(title);
    chapters.push_back({"", title, get_content(urls, translation, proxy)});
    bar.tick();
  }

  kepub::generate_txt(book_info, chapters);
  klib::info("Novel '{}' download completed", book_info.name_);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
