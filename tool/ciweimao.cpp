#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <klib/error.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include "trans.h"
#include "util.h"

namespace {

constexpr std::int32_t ok = 100000;

const std::string app_version = "2.9.100";
const std::string device_token = "ciweimao_client";
const std::string user_agent = "Android com.kuangxiangciweimao.novel";

const std::string default_key = "zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn";
const std::vector<std::uint8_t> iv = {0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0};

const std::string token_path = std::string(std::getenv("HOME")) + "/.ciweimao";

std::string encrypt(const std::string &str) {
  static const auto key = klib::sha_256_raw(default_key);
  return klib::base64_encode(klib::aes_256_cbc_encrypt(str, key, iv));
}

std::string decrypt(const std::string &str) {
  static const auto key = klib::sha_256_raw(default_key);
  return klib::aes_256_cbc_decrypt(klib::base64_decode(str), key, iv);
}

std::string decrypt(const std::string &str, const std::string &key) {
  return klib::aes_256_cbc_decrypt(klib::base64_decode(str),
                                   klib::sha_256_raw(key), iv);
}

klib::Response http_get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url, params, {}, false);
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("HTTP GET fail: {}", response.status_code());
  }

  return response;
}

auto parse_json(const std::string &json, bool check = true) {
  static boost::json::error_code error_code;
  static boost::json::monotonic_resource mr;
  auto jv = boost::json::parse(json, error_code, &mr);
  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  if (check) {
    if (std::string code = jv.at("code").as_string().c_str();
        std::stoi(code) != ok) {
      klib::error(jv.at("tip").as_string().c_str());
    }
  }

  return jv;
}

bool show_user_info(const std::string &account,
                    const std::string &login_token) {
  auto response = http_get("https://app.hbooker.com/reader/get_my_info",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token}});
  auto jv = parse_json(decrypt(response.text()), false);

  if (std::string code = jv.at("code").as_string().c_str();
      std::stoi(code) != ok) {
    // TODO
    return false;
  } else {
    std::string reader_name =
        jv.at("data").at("reader_info").at("reader_name").as_string().c_str();
    spdlog::info("Use existing login token, reader name: {}", reader_name);

    return true;
  }
}

std::optional<std::pair<std::string, std::string>> try_read_token() {
  if (!std::filesystem::exists(token_path)) {
    return {};
  }

  auto json = klib::read_file(token_path, false);
  auto obj = parse_json(decrypt(json), false).as_object();

  std::string account = obj.at("account").as_string().c_str();
  std::string login_token = obj.at("login_token").as_string().c_str();

  show_user_info(account, login_token);

  return {{account, login_token}};
}

void write_token(const std::string &account, const std::string &login_token) {
  boost::json::object obj;
  obj["account"] = account;
  obj["login_token"] = login_token;

  klib::write_file(token_path, false, encrypt(boost::json::serialize(obj)));
}

std::pair<std::string, std::string> login(const std::string &login_name,
                                          const std::string &password) {
  auto response = http_get("https://app.hbooker.com/signup/login",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"login_name", login_name},
                            {"passwd", password}});
  auto jv = parse_json(decrypt(response.text()));

  auto data = jv.at("data").as_object();
  std::string account =
      data.at("reader_info").at("account").as_string().c_str();
  std::string login_token = data.at("login_token").as_string().c_str();

  std::string reader_name =
      data.at("reader_info").at("reader_name").as_string().c_str();
  spdlog::info("Login successful, reader name: {}", reader_name);

  return {account, login_token};
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_get("https://app.hbooker.com/book/get_info_by_id",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"book_id", book_id}});
  auto jv = parse_json(decrypt(response.text()));

  auto book_info = jv.at("data").at("book_info");
  std::string book_name =
      kepub::trans_str(book_info.at("book_name").as_string().c_str());
  std::string author =
      kepub::trans_str(book_info.at("author_name").as_string().c_str());
  std::string description_str = book_info.at("description").as_string().c_str();
  std::string cover_url = book_info.at("cover").as_string().c_str();

  std::vector<std::string> description;
  for (const auto &line : klib::split_str(description_str, "\n")) {
    kepub::push_back(description, kepub::trans_str(line), false);
  }

  spdlog::info("Book name: {}", book_name);
  spdlog::info("Author: {}", author);
  spdlog::info("Cover url: {}", cover_url);

  return {book_name, author, description};
}

std::vector<std::pair<std::string, std::string>> get_book_volume(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_get("https://app.hbooker.com/book/get_division_list",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"book_id", book_id}});
  auto jv = parse_json(decrypt(response.text()));

  std::vector<std::pair<std::string, std::string>> result;

  auto volume_list = jv.at("data").at("division_list").as_array();
  for (const auto &volume : volume_list) {
    std::string volume_id = volume.at("division_id").as_string().c_str();
    std::string volume_name =
        kepub::trans_str(volume.at("division_name").as_string().c_str());

    result.emplace_back(volume_id, volume_name);
  }

  return result;
}

std::vector<std::tuple<std::string, std::string, std::string>> get_chapters(
    const std::string &account, const std::string &login_token,
    const std::string &volume_id, const std::string &volume_title,
    bool download_without_authorization) {
  auto response = http_get(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_id",
      {{"app_version", app_version},
       {"device_token", device_token},
       {"account", account},
       {"login_token", login_token},
       {"division_id", volume_id}});
  auto jv = parse_json(decrypt(response.text()));

  std::vector<std::tuple<std::string, std::string, std::string>> result;

  auto chapter_list = jv.at("data").at("chapter_list").as_array();
  for (const auto &chapter : chapter_list) {
    std::string chapter_id = chapter.at("chapter_id").as_string().c_str();
    std::string chapter_title =
        kepub::trans_str(chapter.at("chapter_title").as_string().c_str());
    std::string is_valid = chapter.at("is_valid").as_string().c_str();
    std::string auth_access = chapter.at("auth_access").as_string().c_str();

    if (is_valid != "1") {
      klib::warn("The chapter is not valid, id: {}, title: {}", chapter_id,
                 chapter_title);
      continue;
    }

    if (auth_access != "1" && !download_without_authorization) {
      klib::warn("No authorized access, id: {}, title: {}", chapter_id,
                 chapter_title);
    } else {
      result.emplace_back(chapter_id, chapter_title, "");
    }
  }

  spdlog::info("Successfully obtained sub-volume: {}", volume_title);

  return result;
}

std::string get_chapter_command(const std::string &account,
                                const std::string &login_token,
                                const std::string &chapter_id) {
  auto response = http_get("https://app.hbooker.com/chapter/get_chapter_cmd",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"chapter_id", chapter_id}});
  auto jv = parse_json(decrypt(response.text()));

  return jv.at("data").at("command").as_string().c_str();
}

std::vector<std::string> get_content(const std::string &account,
                                     const std::string &login_token,
                                     const std::string &chapter_id,
                                     const std::string &chapter_title) {
  auto chapter_command = get_chapter_command(account, login_token, chapter_id);
  auto response = http_get("https://app.hbooker.com/chapter/get_cpt_ifm",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"chapter_id", chapter_id},
                            {"chapter_command", chapter_command}});
  auto jv = parse_json(decrypt(response.text()));
  auto chapter_info = jv.at("data").at("chapter_info");

  std::string encrypt_content_str =
      chapter_info.at("txt_content").as_string().c_str();
  auto content_str = decrypt(encrypt_content_str, chapter_command);

  std::vector<std::string> content;
  for (const auto &line : klib::split_str(content_str, "\n")) {
    kepub::push_back(content, kepub::trans_str(line), false);
  }

  spdlog::info("Successfully obtained chapter: {}", chapter_title);

  return content;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  std::string account, login_token;
  if (auto token = try_read_token(); token.has_value()) {
    std::tie(account, login_token) = *token;
  } else {
    auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    std::tie(account, login_token) = login(login_name, password);
    write_token(account, login_token);
  }

  auto [book_name, author, description] =
      get_book_info(account, login_token, book_id);

  std::vector<
      std::pair<std::string,
                std::vector<std::tuple<std::string, std::string, std::string>>>>
      volume_chapter;
  for (const auto &[volume_id, volume_name] :
       get_book_volume(account, login_token, book_id)) {
    auto chapters = get_chapters(account, login_token, volume_id, volume_name,
                                 options.download_without_authorization_);

    volume_chapter.emplace_back(volume_name, chapters);
  }

  for (auto &[volume_name, chapters] : volume_chapter) {
    for (auto &chapter : chapters) {
      std::get<2>(chapter) =
          boost::join(get_content(account, login_token, std::get<0>(chapter),
                                  std::get<1>(chapter)),
                      "\n");
    }
  }

  std::ofstream book_ofs(book_name + ".txt");
  book_ofs << author << "\n\n";
  for (const auto &line : description) {
    book_ofs << line << "\n";
  }
  book_ofs << "\n";

  for (const auto &[volume_name, chapters] : volume_chapter) {
    book_ofs << "[VOLUME] " << volume_name << "\n\n";

    for (const auto &[chapter_id, chapter_title, content] : chapters) {
      book_ofs << "[WEB] " << chapter_title << "\n\n";
      book_ofs << content << "\n\n";
    }
  }
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
