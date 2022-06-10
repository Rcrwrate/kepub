#include "geetest.h"

#include <sys/utsname.h>
#include <unistd.h>
#include <wait.h>

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread>

#include <httplib.h>
#include <klib/log.h>
#include <klib/url.h>
#include <klib/util.h>
#include <boost/algorithm/string.hpp>

#include "http.h"
#include "json.h"

extern char geetest_js[];
extern int geetest_js_size;

extern char index_html[];
extern int index_html_size;

namespace kepub::ciweimao {

namespace {

GeetestInfo get_geetest_info(const std::string& login_name) {
  klib::URL url("https://app.hbooker.com/signup/geetest_first_register");
  url.set_query(
      {{"t", std::to_string(std::time(nullptr))}, {"user_id", login_name}});
  auto response = http_get_geetest(url.to_string());
  return json_to_geetest_info(response);
}

void open_browser(const std::string& url) {
  utsname name;
  if (uname(&name) == -1) {
    klib::error("uname() failed: {}", std::strerror(errno));
  }

  // WSL
  if (std::string(name.release).find("microsoft") != std::string::npos) {
    klib::exec("powershell.exe /c start " + url);
  } else {
    klib::exec("xdg-open " + url);
  }
}

}  // namespace

std::pair<std::string, std::string> get_geetest_validate(
    const std::string& user_id) {
  auto geetest_info = get_geetest_info(user_id);

  std::string changed_index(index_html, index_html_size);
  boost::replace_all(changed_index, "@gt@", geetest_info.gt_);
  boost::replace_all(changed_index, "@challenge@", geetest_info.challenge_);
  boost::replace_all(changed_index, "@new_captcha@",
                     geetest_info.new_captcha_ ? "true" : "false");

  std::string validate;
  httplib::Server server;

  server.Get("/captcha", [&](const httplib::Request&, httplib::Response& res) {
    res.set_content(changed_index, "text/html");
  });
  server.Get("/geetest.js",
             [&](const httplib::Request&, httplib::Response& res) {
               res.set_content(geetest_js, geetest_js_size, "text/javascript");
             });
  server.Get(R"(/validate/(.+))", [&](const httplib::Request& req,
                                      httplib::Response& res) {
    validate = req.matches[1].str();
    res.set_content("Verification is successful, you can close the browser now",
                    "text/plain");
    server.stop();
  });

  auto pid = fork();
  if (pid < 0) {
    klib::error("fork() failed: {}", strerror(errno));
  }

  if (pid == 0) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    open_browser("http://localhost:3000/captcha");

    std::exit(EXIT_SUCCESS);
  } else {
    server.listen("localhost", 3000);

    if (waitpid(pid, nullptr, 0) == -1) {
      klib::error("waitpid() failed: {}", strerror(errno));
    }
  }

  return {geetest_info.challenge_, validate};
}

}  // namespace kepub::ciweimao
