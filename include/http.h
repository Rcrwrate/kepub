#pragma once

#include <string>

#include <parallel_hashmap/phmap.h>

#include "kepub_export.h"

namespace kepub {

namespace esjzone {

std::string KEPUB_EXPORT http_get(const std::string &url,
                                  const std::string &proxy);

}  // namespace esjzone

namespace lightnovel {

std::string KEPUB_EXPORT http_get(const std::string &url,
                                  const std::string &proxy);

std::string KEPUB_EXPORT http_get_rss(const std::string &url,
                                      const std::string &proxy);

}  // namespace lightnovel

namespace masiro {

std::string KEPUB_EXPORT http_get(const std::string &url,
                                  const std::string &proxy);

std::string KEPUB_EXPORT
http_post(const std::string &url,
          const phmap::flat_hash_map<std::string, std::string> &data,
          const phmap::flat_hash_map<std::string, std::string> &headers,
          const std::string &proxy);

}  // namespace masiro

namespace ciweimao {

std::string KEPUB_EXPORT http_get_rss(const std::string &url);

std::string KEPUB_EXPORT
http_post(const std::string &url,
          phmap::flat_hash_map<std::string, std::string> data);

}  // namespace ciweimao

namespace sfacg {

std::string KEPUB_EXPORT http_get(const std::string &url);

std::string KEPUB_EXPORT http_get_rss(const std::string &url);

std::string KEPUB_EXPORT http_post(const std::string &url,
                                   const std::string &json);

}  // namespace sfacg

}  // namespace kepub
