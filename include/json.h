#pragma once

#include <string>
#include <vector>

#include "kepub_export.h"
#include "novel.h"

namespace kepub {

namespace masiro {

void KEPUB_EXPORT json_base(std::string json);

}  // namespace masiro

namespace lightnovel {

std::string KEPUB_EXPORT login_serialize(const std::string &login_name,
                                         const std::string &password);

std::string KEPUB_EXPORT http_post_serialize(const std::string &security_key);

struct KEPUB_EXPORT UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_EXPORT json_to_user_info(std::string json);

struct KEPUB_EXPORT LoginInfo {
  std::string security_key_;
  UserInfo user_info_;
};

LoginInfo KEPUB_EXPORT json_to_login_info(std::string json);

}  // namespace lightnovel

namespace ciweimao {

std::string KEPUB_EXPORT serialize(const std::string &account,
                                   const std::string &login_token);

struct KEPUB_EXPORT Token {
  std::string account_;
  std::string login_token_;
};

Token KEPUB_EXPORT json_to_token(std::string json);

struct KEPUB_EXPORT UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_EXPORT json_to_user_info(std::string json);

struct KEPUB_EXPORT LoginInfo {
  Token token_;
  UserInfo user_info_;
};

LoginInfo KEPUB_EXPORT json_to_login_info(std::string json);

kepub::BookInfo KEPUB_EXPORT json_to_book_info(std::string json);

std::vector<kepub::Volume> KEPUB_EXPORT json_to_volumes(std::string json);

std::string KEPUB_EXPORT json_to_chapter_command(std::string json);

std::string KEPUB_EXPORT json_to_chapter_text(std::string json);

}  // namespace ciweimao

namespace sfacg {

std::string KEPUB_EXPORT serialize(const std::string &login_name,
                                   const std::string &password);

void KEPUB_EXPORT json_base(std::string json);

struct KEPUB_EXPORT UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_EXPORT json_to_user_info(std::string json);

struct KEPUB_EXPORT LoginInfo {
  UserInfo user_info_;
};

LoginInfo KEPUB_EXPORT json_to_login_info(std::string json);

kepub::BookInfo KEPUB_EXPORT json_to_book_info(std::string json);

std::vector<kepub::Volume> KEPUB_EXPORT json_to_volumes(std::string json);

std::string KEPUB_EXPORT json_to_chapter_text(std::string json);

}  // namespace sfacg

}  // namespace kepub
