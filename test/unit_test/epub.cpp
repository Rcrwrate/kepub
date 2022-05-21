#include <filesystem>
#include <memory>

#include <fmt/format.h>
#include <klib/util.h>
#include <catch2/catch_test_macros.hpp>

#include "epub.h"

TEST_CASE("base generate", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book1";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book1"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book1");

  CHECK(std::filesystem::exists(kepub::Epub::container_xml_path));
  CHECK(klib::read_file(kepub::Epub::container_xml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles>
    <rootfile full-path="EPUB/package.opf" media-type="application/oebps-package+xml" />
  </rootfiles>
</container>
)");

  CHECK(std::filesystem::exists(kepub::Epub::font_woff2_path));
  CHECK(std::filesystem::file_size(kepub::Epub::font_woff2_path) == 2452);

  CHECK(std::filesystem::exists(kepub::Epub::style_css_path));
  CHECK(std::filesystem::file_size(kepub::Epub::style_css_path) == 729);

  CHECK(std::filesystem::exists(kepub::Epub::introduction_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::introduction_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>简介</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body epub:type="introduction">
    <div>
      <h1>简介</h1>
      <p>test</p>
      <p>introduction</p>
    </div>
  </body>
</html>
)");

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book1</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book1</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="introduction.xhtml" />
  </spine>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  CHECK(std::filesystem::exists(kepub::Epub::mimetype_path));
  CHECK(klib::read_file(kepub::Epub::mimetype_path, false) ==
        R"(application/epub+zip)");

  ptr.reset();

  std::filesystem::remove_all("test book1");
}

TEST_CASE("generate postscript", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book2";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.postscript_ = {"postscript"};

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book2"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book2");

  CHECK(std::filesystem::exists(kepub::Epub::postscript_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::postscript_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>后记</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body epub:type="afterword">
    <div>
      <h1>后记</h1>
      <p>postscript</p>
    </div>
  </body>
</html>
)");

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book2</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book2</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="postscript.xhtml" href="text/postscript.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="introduction.xhtml" />
    <itemref idref="postscript.xhtml" />
  </spine>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/postscript.xhtml">后记</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book2");
}

TEST_CASE("generate postscript and cover", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book3";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.postscript_ = {"postscript"};
  novel.book_info_.cover_path_ = "cover.jpg";

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book3"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book3");

  CHECK(std::filesystem::exists(kepub::Epub::cover_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::cover_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>封面</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body epub:type="coverpage">
    <div class="cover">
      <img alt="cover" src="../image/cover.webp" />
    </div>
  </body>
</html>
)");

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book3</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book3</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
    <meta name="cover" content="cover.webp" />
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="cover.webp" href="image/cover.webp" media-type="image/webp" properties="cover-image" />
    <item id="cover.xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="postscript.xhtml" href="text/postscript.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="cover.xhtml" />
    <itemref idref="introduction.xhtml" />
    <itemref idref="postscript.xhtml" />
  </spine>
  <guide>
    <reference type="cover" title="封面" href="text/cover.xhtml" />
  </guide>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/cover.xhtml">封面</a>
        </li>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/postscript.xhtml">后记</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book3");
}

TEST_CASE("generate postscript, cover, illustration and image", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book4";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.postscript_ = {"postscript"};
  novel.book_info_.cover_path_ = "cover.jpg";
  novel.illustration_num_ = 3;
  novel.image_paths_ = {"001.jpg", "002.jpg", "003.jpg", "004.jpg", "005.jpg"};

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book4"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book4");

  for (std::int32_t i = 1; i <= 3; ++i) {
    auto file_name = "illustration00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    CHECK(std::filesystem::exists(path));
    CHECK(klib::read_file(path, false) ==
          fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>彩页 {}</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body>
    <div class="center">
      <img alt="00{}" src="../image/00{}.webp" />
    </div>
  </body>
</html>
)",
                      std::to_string(i), std::to_string(i), std::to_string(i)));
  }

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book4</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book4</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
    <meta name="cover" content="cover.webp" />
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="x001.webp" href="image/001.webp" media-type="image/webp" />
    <item id="x002.webp" href="image/002.webp" media-type="image/webp" />
    <item id="x003.webp" href="image/003.webp" media-type="image/webp" />
    <item id="x004.webp" href="image/004.webp" media-type="image/webp" />
    <item id="x005.webp" href="image/005.webp" media-type="image/webp" />
    <item id="cover.webp" href="image/cover.webp" media-type="image/webp" properties="cover-image" />
    <item id="cover.xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration001.xhtml" href="text/illustration001.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration002.xhtml" href="text/illustration002.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration003.xhtml" href="text/illustration003.xhtml" media-type="application/xhtml+xml" />
    <item id="postscript.xhtml" href="text/postscript.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="cover.xhtml" />
    <itemref idref="introduction.xhtml" />
    <itemref idref="illustration001.xhtml" />
    <itemref idref="illustration002.xhtml" />
    <itemref idref="illustration003.xhtml" />
    <itemref idref="postscript.xhtml" />
  </spine>
  <guide>
    <reference type="cover" title="封面" href="text/cover.xhtml" />
  </guide>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/cover.xhtml">封面</a>
        </li>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/illustration001.xhtml">彩页 1</a>
        </li>
        <li>
          <a href="text/illustration002.xhtml">彩页 2</a>
        </li>
        <li>
          <a href="text/illustration003.xhtml">彩页 3</a>
        </li>
        <li>
          <a href="text/postscript.xhtml">后记</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book4");
}

TEST_CASE("full generate", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book5";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.postscript_ = {"postscript"};
  novel.book_info_.cover_path_ = "cover.jpg";
  novel.illustration_num_ = 3;
  novel.image_paths_ = {"001.jpg", "002.jpg", "003.jpg", "004.jpg", "005.jpg"};

  novel.volumes_.emplace_back(std::vector<kepub::Chapter>{
      kepub::Chapter("title 1", std::vector<std::string>{"abc 1"}),
      kepub::Chapter("title 2", std::vector<std::string>{"abc 2"}),
      kepub::Chapter("title 3", std::vector<std::string>{"abc 3"})});

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book5"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book5");

  for (std::int32_t i = 1; i <= 3; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    CHECK(std::filesystem::exists(path));
    CHECK(klib::read_file(path, false) ==
          fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>title {}</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body>
    <div>
      <h1>title {}</h1>
      <p>abc {}</p>
    </div>
  </body>
</html>
)",
                      std::to_string(i), std::to_string(i), std::to_string(i)));
  }

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book5</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book5</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
    <meta name="cover" content="cover.webp" />
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="x001.webp" href="image/001.webp" media-type="image/webp" />
    <item id="x002.webp" href="image/002.webp" media-type="image/webp" />
    <item id="x003.webp" href="image/003.webp" media-type="image/webp" />
    <item id="x004.webp" href="image/004.webp" media-type="image/webp" />
    <item id="x005.webp" href="image/005.webp" media-type="image/webp" />
    <item id="cover.webp" href="image/cover.webp" media-type="image/webp" properties="cover-image" />
    <item id="cover.xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration001.xhtml" href="text/illustration001.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration002.xhtml" href="text/illustration002.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration003.xhtml" href="text/illustration003.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter001.xhtml" href="text/chapter001.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter002.xhtml" href="text/chapter002.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter003.xhtml" href="text/chapter003.xhtml" media-type="application/xhtml+xml" />
    <item id="postscript.xhtml" href="text/postscript.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="cover.xhtml" />
    <itemref idref="introduction.xhtml" />
    <itemref idref="illustration001.xhtml" />
    <itemref idref="illustration002.xhtml" />
    <itemref idref="illustration003.xhtml" />
    <itemref idref="chapter001.xhtml" />
    <itemref idref="chapter002.xhtml" />
    <itemref idref="chapter003.xhtml" />
    <itemref idref="postscript.xhtml" />
  </spine>
  <guide>
    <reference type="cover" title="封面" href="text/cover.xhtml" />
  </guide>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/cover.xhtml">封面</a>
        </li>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/illustration001.xhtml">彩页 1</a>
        </li>
        <li>
          <a href="text/illustration002.xhtml">彩页 2</a>
        </li>
        <li>
          <a href="text/illustration003.xhtml">彩页 3</a>
        </li>
        <li>
          <a href="text/chapter001.xhtml">title 1</a>
        </li>
        <li>
          <a href="text/chapter002.xhtml">title 2</a>
        </li>
        <li>
          <a href="text/chapter003.xhtml">title 3</a>
        </li>
        <li>
          <a href="text/postscript.xhtml">后记</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book5");
}

TEST_CASE("sub-volume", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book6";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.postscript_ = {"postscript"};
  novel.book_info_.cover_path_ = "cover.jpg";
  novel.illustration_num_ = 3;
  novel.image_paths_ = {"001.jpg", "002.jpg", "003.jpg", "004.jpg", "005.jpg"};

  novel.volumes_.emplace_back(
      "volume 1", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 1", std::vector<std::string>{"abc 1"})});
  novel.volumes_.emplace_back(
      "volume 2", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 2", std::vector<std::string>{"abc 2"})});
  novel.volumes_.emplace_back(
      "volume 3", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 3", std::vector<std::string>{"abc 3"})});

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book6"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book6");

  for (std::int32_t i = 1; i <= 3; ++i) {
    auto file_name = "volume00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    CHECK(std::filesystem::exists(path));
    CHECK(klib::read_file(path, false) ==
          fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>volume {}</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body>
    <div>
      <h1>volume {}</h1>
    </div>
  </body>
</html>
)",
                      std::to_string(i), std::to_string(i)));
  }

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book6</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book6</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
    <meta name="cover" content="cover.webp" />
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="x001.webp" href="image/001.webp" media-type="image/webp" />
    <item id="x002.webp" href="image/002.webp" media-type="image/webp" />
    <item id="x003.webp" href="image/003.webp" media-type="image/webp" />
    <item id="x004.webp" href="image/004.webp" media-type="image/webp" />
    <item id="x005.webp" href="image/005.webp" media-type="image/webp" />
    <item id="cover.webp" href="image/cover.webp" media-type="image/webp" properties="cover-image" />
    <item id="cover.xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration001.xhtml" href="text/illustration001.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration002.xhtml" href="text/illustration002.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration003.xhtml" href="text/illustration003.xhtml" media-type="application/xhtml+xml" />
    <item id="volume001.xhtml" href="text/volume001.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter001.xhtml" href="text/chapter001.xhtml" media-type="application/xhtml+xml" />
    <item id="volume002.xhtml" href="text/volume002.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter002.xhtml" href="text/chapter002.xhtml" media-type="application/xhtml+xml" />
    <item id="volume003.xhtml" href="text/volume003.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter003.xhtml" href="text/chapter003.xhtml" media-type="application/xhtml+xml" />
    <item id="postscript.xhtml" href="text/postscript.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="cover.xhtml" />
    <itemref idref="introduction.xhtml" />
    <itemref idref="illustration001.xhtml" />
    <itemref idref="illustration002.xhtml" />
    <itemref idref="illustration003.xhtml" />
    <itemref idref="volume001.xhtml" />
    <itemref idref="chapter001.xhtml" />
    <itemref idref="volume002.xhtml" />
    <itemref idref="chapter002.xhtml" />
    <itemref idref="volume003.xhtml" />
    <itemref idref="chapter003.xhtml" />
    <itemref idref="postscript.xhtml" />
  </spine>
  <guide>
    <reference type="cover" title="封面" href="text/cover.xhtml" />
  </guide>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/cover.xhtml">封面</a>
        </li>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/illustration001.xhtml">彩页 1</a>
        </li>
        <li>
          <a href="text/illustration002.xhtml">彩页 2</a>
        </li>
        <li>
          <a href="text/illustration003.xhtml">彩页 3</a>
        </li>
        <li>
          <a href="text/volume001.xhtml">volume 1</a>
          <ol>
            <li>
              <a href="text/chapter001.xhtml">title 1</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume002.xhtml">volume 2</a>
          <ol>
            <li>
              <a href="text/chapter002.xhtml">title 2</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume003.xhtml">volume 3</a>
          <ol>
            <li>
              <a href="text/chapter003.xhtml">title 3</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/postscript.xhtml">后记</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book6");
}

TEST_CASE("append", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book7";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.postscript_ = {"postscript"};
  novel.book_info_.cover_path_ = "cover.jpg";
  novel.illustration_num_ = 3;
  novel.image_paths_ = {"001.jpg", "002.jpg", "003.jpg", "004.jpg", "005.jpg"};

  novel.volumes_.emplace_back(
      "volume 1", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 1", std::vector<std::string>{"abc 1"})});
  novel.volumes_.emplace_back(
      "volume 2", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 2", std::vector<std::string>{"abc 2"})});
  novel.volumes_.emplace_back(
      "volume 3", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 3", std::vector<std::string>{"abc 3"})});

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book7"));

  novel.volumes_.clear();
  novel.volumes_.emplace_back(std::vector<kepub::Chapter>{
      kepub::Chapter("title 4", std::vector<std::string>{"abc 4"}),
      kepub::Chapter("title 5", std::vector<std::string>{"abc 5"})});

  epub.set_novel(novel);
  CHECK_NOTHROW(epub.append());

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book7");

  for (std::int32_t i = 4; i <= 5; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    CHECK(std::filesystem::exists(path));
    CHECK(klib::read_file(path, false) ==
          fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>title {}</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body>
    <div>
      <h1>title {}</h1>
      <p>abc {}</p>
    </div>
  </body>
</html>
)",
                      std::to_string(i), std::to_string(i), std::to_string(i)));
  }

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book7</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book7</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
    <meta name="cover" content="cover.webp" />
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="x001.webp" href="image/001.webp" media-type="image/webp" />
    <item id="x002.webp" href="image/002.webp" media-type="image/webp" />
    <item id="x003.webp" href="image/003.webp" media-type="image/webp" />
    <item id="x004.webp" href="image/004.webp" media-type="image/webp" />
    <item id="x005.webp" href="image/005.webp" media-type="image/webp" />
    <item id="cover.webp" href="image/cover.webp" media-type="image/webp" properties="cover-image" />
    <item id="cover.xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration001.xhtml" href="text/illustration001.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration002.xhtml" href="text/illustration002.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration003.xhtml" href="text/illustration003.xhtml" media-type="application/xhtml+xml" />
    <item id="volume001.xhtml" href="text/volume001.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter001.xhtml" href="text/chapter001.xhtml" media-type="application/xhtml+xml" />
    <item id="volume002.xhtml" href="text/volume002.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter002.xhtml" href="text/chapter002.xhtml" media-type="application/xhtml+xml" />
    <item id="volume003.xhtml" href="text/volume003.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter003.xhtml" href="text/chapter003.xhtml" media-type="application/xhtml+xml" />
    <item id="postscript.xhtml" href="text/postscript.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter004.xhtml" href="text/chapter004.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter005.xhtml" href="text/chapter005.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="cover.xhtml" />
    <itemref idref="introduction.xhtml" />
    <itemref idref="illustration001.xhtml" />
    <itemref idref="illustration002.xhtml" />
    <itemref idref="illustration003.xhtml" />
    <itemref idref="volume001.xhtml" />
    <itemref idref="chapter001.xhtml" />
    <itemref idref="volume002.xhtml" />
    <itemref idref="chapter002.xhtml" />
    <itemref idref="volume003.xhtml" />
    <itemref idref="chapter003.xhtml" />
    <itemref idref="postscript.xhtml" />
    <itemref idref="chapter004.xhtml" />
    <itemref idref="chapter005.xhtml" />
  </spine>
  <guide>
    <reference type="cover" title="封面" href="text/cover.xhtml" />
  </guide>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/cover.xhtml">封面</a>
        </li>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/illustration001.xhtml">彩页 1</a>
        </li>
        <li>
          <a href="text/illustration002.xhtml">彩页 2</a>
        </li>
        <li>
          <a href="text/illustration003.xhtml">彩页 3</a>
        </li>
        <li>
          <a href="text/volume001.xhtml">volume 1</a>
          <ol>
            <li>
              <a href="text/chapter001.xhtml">title 1</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume002.xhtml">volume 2</a>
          <ol>
            <li>
              <a href="text/chapter002.xhtml">title 2</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume003.xhtml">volume 3</a>
          <ol>
            <li>
              <a href="text/chapter003.xhtml">title 3</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/postscript.xhtml">后记</a>
        </li>
        <li>
          <a href="text/chapter004.xhtml">title 4</a>
        </li>
        <li>
          <a href="text/chapter005.xhtml">title 5</a>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book7");
}

TEST_CASE("append sub-volume", "[epub]") {
  kepub::Epub epub;
  epub.set_rights("Kaiser");

  kepub::Novel novel;
  novel.book_info_.name_ = "test book8";
  novel.book_info_.author_ = "test author";
  novel.book_info_.introduction_ = {"test", "introduction"};
  novel.book_info_.cover_path_ = "cover.jpg";
  novel.illustration_num_ = 3;
  novel.image_paths_ = {"001.jpg", "002.jpg", "003.jpg", "004.jpg", "005.jpg"};

  novel.volumes_.emplace_back(
      "volume 1", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 1", std::vector<std::string>{"abc 1"})});
  novel.volumes_.emplace_back(
      "volume 2", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 2", std::vector<std::string>{"abc 2"})});
  novel.volumes_.emplace_back(
      "volume 3", std::vector<kepub::Chapter>{kepub::Chapter(
                      "title 3", std::vector<std::string>{"abc 3"})});

  epub.set_novel(novel);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_datetime("2021-08-01");

  CHECK_NOTHROW(epub.generate());
  CHECK(std::filesystem::is_directory("test book8"));

  novel.volumes_.clear();
  novel.volumes_.emplace_back(std::vector<kepub::Chapter>{
      kepub::Chapter("title 4", std::vector<std::string>{"abc 4"})});
  novel.volumes_.emplace_back(
      "volume 4",
      std::vector<kepub::Chapter>{
          kepub::Chapter("title 5", std::vector<std::string>{"abc 5"}),
          kepub::Chapter("title 6", std::vector<std::string>{"abc 6"})});

  epub.set_novel(novel);
  CHECK_NOTHROW(epub.append());

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book8");

  for (std::int32_t i = 4; i <= 4; ++i) {
    auto file_name = "volume00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    CHECK(std::filesystem::exists(path));
    CHECK(klib::read_file(path, false) ==
          fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>volume {}</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body>
    <div>
      <h1>volume {}</h1>
    </div>
  </body>
</html>
)",
                      std::to_string(i), std::to_string(i)));
  }

  for (std::int32_t i = 4; i <= 6; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    CHECK(std::filesystem::exists(path));
    CHECK(klib::read_file(path, false) ==
          fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>title {}</title>
    <link rel="stylesheet" href="../css/style.css" />
  </head>
  <body>
    <div>
      <h1>title {}</h1>
      <p>abc {}</p>
    </div>
  </body>
</html>
)",
                      std::to_string(i), std::to_string(i), std::to_string(i)));
  }

  CHECK(std::filesystem::exists(kepub::Epub::package_opf_path));
  CHECK(klib::read_file(kepub::Epub::package_opf_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" xml:lang="zh-CN" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    <meta refines="#pub-id" property="identifier-type" scheme="xsd:string">uuid</meta>
    <dc:title id="title">test book8</dc:title>
    <meta refines="#title" property="title-type">main</meta>
    <meta refines="#title" property="file-as">test book8</meta>
    <dc:creator id="creator">test author</dc:creator>
    <meta refines="#creator" property="role" scheme="marc:relators">aut</meta>
    <meta refines="#creator" property="file-as">test author</meta>
    <dc:language>zh-CN</dc:language>
    <dc:rights>Kaiser</dc:rights>
    <dc:description>test
introduction</dc:description>
    <meta property="dcterms:modified">2021-08-01</meta>
    <meta name="cover" content="cover.webp" />
  </metadata>
  <manifest>
    <item id="style.css" href="css/style.css" media-type="text/css" />
    <item id="SourceHanSansSC-Bold.woff2" href="font/SourceHanSansSC-Bold.woff2" media-type="font/woff2" />
    <item id="x001.webp" href="image/001.webp" media-type="image/webp" />
    <item id="x002.webp" href="image/002.webp" media-type="image/webp" />
    <item id="x003.webp" href="image/003.webp" media-type="image/webp" />
    <item id="x004.webp" href="image/004.webp" media-type="image/webp" />
    <item id="x005.webp" href="image/005.webp" media-type="image/webp" />
    <item id="cover.webp" href="image/cover.webp" media-type="image/webp" properties="cover-image" />
    <item id="cover.xhtml" href="text/cover.xhtml" media-type="application/xhtml+xml" />
    <item id="nav.xhtml" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />
    <item id="introduction.xhtml" href="text/introduction.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration001.xhtml" href="text/illustration001.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration002.xhtml" href="text/illustration002.xhtml" media-type="application/xhtml+xml" />
    <item id="illustration003.xhtml" href="text/illustration003.xhtml" media-type="application/xhtml+xml" />
    <item id="volume001.xhtml" href="text/volume001.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter001.xhtml" href="text/chapter001.xhtml" media-type="application/xhtml+xml" />
    <item id="volume002.xhtml" href="text/volume002.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter002.xhtml" href="text/chapter002.xhtml" media-type="application/xhtml+xml" />
    <item id="volume003.xhtml" href="text/volume003.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter003.xhtml" href="text/chapter003.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter004.xhtml" href="text/chapter004.xhtml" media-type="application/xhtml+xml" />
    <item id="volume004.xhtml" href="text/volume004.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter005.xhtml" href="text/chapter005.xhtml" media-type="application/xhtml+xml" />
    <item id="chapter006.xhtml" href="text/chapter006.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine>
    <itemref idref="cover.xhtml" />
    <itemref idref="introduction.xhtml" />
    <itemref idref="illustration001.xhtml" />
    <itemref idref="illustration002.xhtml" />
    <itemref idref="illustration003.xhtml" />
    <itemref idref="volume001.xhtml" />
    <itemref idref="chapter001.xhtml" />
    <itemref idref="volume002.xhtml" />
    <itemref idref="chapter002.xhtml" />
    <itemref idref="volume003.xhtml" />
    <itemref idref="chapter003.xhtml" />
    <itemref idref="chapter004.xhtml" />
    <itemref idref="volume004.xhtml" />
    <itemref idref="chapter005.xhtml" />
    <itemref idref="chapter006.xhtml" />
  </spine>
  <guide>
    <reference type="cover" title="封面" href="text/cover.xhtml" />
  </guide>
</package>
)");

  CHECK(std::filesystem::exists(kepub::Epub::nav_xhtml_path));
  CHECK(klib::read_file(kepub::Epub::nav_xhtml_path, false) ==
        R"(<?xml version="1.0" encoding="UTF-8"?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="zh-CN">
  <head>
    <title>目录</title>
    <link rel="stylesheet" href="css/style.css" />
  </head>
  <body>
    <nav epub:type="toc">
      <ol>
        <li>
          <a href="text/cover.xhtml">封面</a>
        </li>
        <li>
          <a href="text/introduction.xhtml">简介</a>
        </li>
        <li>
          <a href="text/illustration001.xhtml">彩页 1</a>
        </li>
        <li>
          <a href="text/illustration002.xhtml">彩页 2</a>
        </li>
        <li>
          <a href="text/illustration003.xhtml">彩页 3</a>
        </li>
        <li>
          <a href="text/volume001.xhtml">volume 1</a>
          <ol>
            <li>
              <a href="text/chapter001.xhtml">title 1</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume002.xhtml">volume 2</a>
          <ol>
            <li>
              <a href="text/chapter002.xhtml">title 2</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume003.xhtml">volume 3</a>
          <ol>
            <li>
              <a href="text/chapter003.xhtml">title 3</a>
            </li>
            <li>
              <a href="text/chapter004.xhtml">title 4</a>
            </li>
          </ol>
        </li>
        <li>
          <a href="text/volume004.xhtml">volume 4</a>
          <ol>
            <li>
              <a href="text/chapter005.xhtml">title 5</a>
            </li>
            <li>
              <a href="text/chapter006.xhtml">title 6</a>
            </li>
          </ol>
        </li>
      </ol>
    </nav>
  </body>
</html>
)");

  ptr.reset();

  std::filesystem::remove_all("test book8");
}
