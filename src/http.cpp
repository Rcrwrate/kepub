#include "http.h"

#include <cstdint>
#include <ctime>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/hash.h>
#include <klib/http.h>
#include <klib/log.h>
#include <klib/url.h>
#include <klib/util.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

namespace kepub {

namespace {

thread_local klib::Request request;

void report_http_error(klib::HttpStatus status, const std::string &url) {
  klib::error("HTTP GET failed, code: {}, reason: {}, url: {}",
              static_cast<std::int32_t>(status), klib::http_status_str(status),
              url);
}

std::string http_get(const std::string &url, const std::string &proxy) {
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }
  request.set_doh_url("https://dns.google/dns-query");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

std::string http_post(
    const std::string &url,
    const phmap::flat_hash_map<std::string, std::string> &data,
    const phmap::flat_hash_map<std::string, std::string> &headers,
    const std::string &proxy) {
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }
  request.set_doh_url("https://dns.google/dns-query");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.post(url, data, headers);

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

}  // namespace

namespace esjzone {

std::string http_get(const std::string &url, const std::string &proxy) {
  return kepub::http_get(url, proxy);
}

}  // namespace esjzone

namespace lightnovel {

std::string http_post(const std::string &url, const std::string &json,
                      const std::string &proxy) {
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }
  request.set_doh_url("https://dns.google/dns-query");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.post(url, json);

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

std::string http_get(const std::string &url, const std::string &security_key,
                     const std::string &proxy) {
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }

  request.set_doh_url("https://dns.google/dns-query");

  boost::json::object obj;
  obj["security_key"] = security_key;
  request.set_cookie({{"token", boost::json::serialize(obj)}});
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

std::string http_get_rss(const std::string &url, const std::string &proxy) {
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }
  request.set_doh_url("https://dns.google/dns-query");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url, {{"referer", "https://www.lightnovel.us/"}});

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

}  // namespace lightnovel

namespace masiro {

std::string http_get(const std::string &url, const std::string &proxy) {
  return kepub::http_get(url, proxy);
}

std::string http_post(
    const std::string &url,
    const phmap::flat_hash_map<std::string, std::string> &data,
    const phmap::flat_hash_map<std::string, std::string> &headers,
    const std::string &proxy) {
  return kepub::http_post(url, data, headers, proxy);
}

}  // namespace masiro

namespace ciweimao {

namespace {

const std::string device_token = "ciweimao_";

const std::string app_version = "2.9.282";
const std::string app_version_new = "2.9.290";

const std::string user_agent =
    "Android  com.kuangxiangciweimao.novel  2.9.282,OnePlus, ONEPLUS A3010, "
    "25, 7.1.1";
const std::string user_agent_new =
    "Android  com.kuangxiangciweimao.novel  2.9.290,OnePlus, ONEPLUS A3010, "
    "25, 7.1.1";

const static std::string user_agent_rss =
    "Dalvik/2.1.0 (Linux; U; Android 7.1.1; ONEPLUS A3010 Build/NMF26F)";
const static std::string user_agent_geetest = "okhttp/4.8.0";

}  // namespace

std::string http_get_rss(const std::string &url) {
  request.set_no_proxy();
  request.set_user_agent(user_agent_rss);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url, {{"Connection", "keep-alive"}});

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

std::string http_get_geetest(const std::string &url) {
  request.set_no_proxy();
  request.set_user_agent(user_agent_geetest);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url, {{"Connection", "keep-alive"}});

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

std::string http_post(const std::string &url,
                      phmap::flat_hash_map<std::string, std::string> data,
                      bool new_version) {
  request.set_no_proxy();

  if (new_version) {
    request.set_user_agent(user_agent_new);
  } else {
    request.set_user_agent(user_agent);
  }

#ifndef NDEBUG
  request.verbose(true);
#endif

  if (new_version) {
    data.emplace("app_version", app_version_new);
  } else {
    data.emplace("app_version", app_version);
  }

  data.emplace("device_token", device_token);

  auto response = request.post(url, data, {{"Connection", "keep-alive"}});

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response.text();
}

}  // namespace ciweimao

namespace sfacg {

namespace {

const std::string user_name = "apiuser";
const std::string password = "3s#1-yt6e*Acv@qer";
const std::string device_token = "1F6EF324-A916-4995-971D-3AA71813072B";
const std::string user_agent =
    "boluobao/4.8.54(iOS;15.5)/appStore/" + device_token;
const std::string user_agent_rss =
    "SFReader/4.8.54 (iPhone; iOS 15.5; Scale/3.00)";

std::string sf_security() {
  std::string uuid = klib::uuid();
  auto timestamp = std::time(nullptr);
  std::string sign = boost::to_upper_copy(klib::md5_hex(
      uuid + std::to_string(timestamp) + device_token + "FMLxgOdsfxmN!Dt4"));

  return fmt::format(
      FMT_COMPILE("nonce={}&timestamp={}&devicetoken={}&sign={}"), uuid,
      timestamp, device_token, sign);
}

}  // namespace

std::string http_get(const std::string &url) {
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.basic_auth(user_name, password);
#ifndef NDEBUG
  request.verbose(true);
#endif

  return request
      .get(url, {{"Connection", "keep-alive"},
                 {"Accept", "application/vnd.sfacg.api+json;version=1"},
                 {"SFSecurity", sf_security()},
                 {"Accept-Language", "zh-Hans-CN;q=1"}})
      .text();
}

std::string http_get_rss(const std::string &url) {
  request.set_no_proxy();
  request.set_user_agent(user_agent_rss);
#ifndef NDEBUG
  request.verbose(true);
#endif

  return request
      .get(url, {{"Accept", "image/*,*/*;q=0.8"},
                 {"Accept-Language", "zh-CN,zh-Hans;q=0.9"},
                 {"Connection", "keep-alive"}})
      .text();
}

std::string http_post(const std::string &url, const std::string &json) {
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.basic_auth(user_name, password);
#ifndef NDEBUG
  request.verbose(true);
#endif

  return request
      .post(url, json,
            {{"Connection", "keep-alive"},
             {"Accept", "application/vnd.sfacg.api+json;version=1"},
             {"SFSecurity", sf_security()},
             {"Accept-Language", "zh-Hans-CN;q=1"}})
      .text();
}

}  // namespace sfacg

}  // namespace kepub
