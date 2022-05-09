#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "kepub_export.h"
#include "novel.h"

namespace kepub {

std::string KEPUB_EXPORT footer_str();

void KEPUB_EXPORT check_file_exist(const std::string &file_name);

void KEPUB_EXPORT check_is_txt_file(const std::string &file_name);

void KEPUB_EXPORT check_is_epub_file(const std::string &file_name);

void KEPUB_EXPORT remove_file_or_dir(const std::string &path);

void KEPUB_EXPORT check_is_book_id(const std::string &book_id);

std::vector<std::string> KEPUB_EXPORT
read_file_to_vec(const std::string &file_name, bool translation);

void KEPUB_EXPORT str_check(const std::string &str);

std::int32_t KEPUB_EXPORT str_size(const std::string &str);

void KEPUB_EXPORT volume_name_check(const std::string &volume_name);

void KEPUB_EXPORT title_check(const std::string &title);

void KEPUB_EXPORT push_back(std::vector<std::string> &texts,
                            const std::string &str, bool connect,
                            bool check = true);

void KEPUB_EXPORT push_back(std::vector<std::string> &texts,
                            const std::string &str);

std::string KEPUB_EXPORT get_login_name();

std::string KEPUB_EXPORT get_password();

std::string KEPUB_EXPORT num_to_str(std::int32_t i);

std::string KEPUB_EXPORT url_to_stem_name(const std::string &str);

std::optional<std::string> KEPUB_EXPORT
check_is_supported_format(const std::string &file_name);

std::optional<std::string> KEPUB_EXPORT
image_to_extension(const std::string &image);

std::string KEPUB_EXPORT stem(const std::string &path);

std::string KEPUB_EXPORT make_book_name_legal(const std::string &file_name);

void KEPUB_EXPORT generate_txt(const BookInfo &book_info,
                               const std::vector<Chapter> &chapters);

void KEPUB_EXPORT generate_txt(const BookInfo &book_info,
                               const std::vector<Volume> &volumes);

}  // namespace kepub
