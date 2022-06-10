# kepub

[![Build](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml/badge.svg)](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml)
[![Platform](https://img.shields.io/badge/Platform-Debian%2011-brightgreen)](https://www.debian.org/)
[![Coverage Status](https://codecov.io/gh/KaiserLancelot/kepub/branch/main/graph/badge.svg?token=ZR870P8548)](https://codecov.io/gh/KaiserLancelot/kepub)
[![GitHub Releases](https://img.shields.io/github/release/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/KaiserLancelot/kepub/total)](https://github.com/KaiserLancelot/kepub/releases)
[![GitHub License](https://img.shields.io/github/license/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/blob/main/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

---

Crawl novels from sfacg, ciweimao, esjzone, lightnovel and masiro; generate, append and extract epub

## Third party

- klib ([MIT License](https://github.com/KaiserLancelot/klib/blob/main/LICENSE))
- mimalloc ([MIT License](https://github.com/microsoft/mimalloc/blob/master/LICENSE))
- parallel-hashmap ([Apache License 2.0](https://github.com/greg7mdp/parallel-hashmap/blob/master/LICENSE))
- RE2 ([BSD 3-Clause "New" or "Revised" License](https://github.com/google/re2/blob/main/LICENSE))
- oneTBB ([Apache License 2.0](https://github.com/oneapi-src/oneTBB/blob/master/LICENSE.txt))
- Boost ([Boost License](https://www.boost.org/users/license.html))
- GSL ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
- CLI11 ([License](https://github.com/CLIUtils/CLI11/blob/main/LICENSE))
- fmt ([License](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst))
- simdjson ([Apache License 2.0](https://github.com/simdjson/simdjson/blob/master/LICENSE))
- pugixml ([MIT License](https://github.com/zeux/pugixml/blob/master/LICENSE.md))
- OpenCC ([Apache License 2.0](https://github.com/BYVoid/OpenCC/blob/master/LICENSE))
- indicators ([MIT License](https://github.com/p-ranav/indicators/blob/master/LICENSE))
- cpp-httplib ([MIT License](https://github.com/yhirose/cpp-httplib/blob/master/LICENSE))

## Font

- Source Han Sans ([SIL Open Font License](https://github.com/adobe-fonts/source-han-sans/blob/master/LICENSE.txt))

## Build environment

- [kenv](https://github.com/KaiserLancelot/kenv)

## Usage

- Make sure you are using a recent version of Debian/Ubuntu/WSL2 (other Linux systems have not been tested, but should work)
- The program requires AVX2 instruction set, the CPU is at least Intel Haswell (2013), if you use a virtual machine, you need additional settings about the CPU
- Install the DEB package (or use the compressed package) or compile it yourself
- Then you can start using, for example:

```bash
sfacg book-id
gen-epub book-name.txt
```

## Roadmap

- Rewritten in Rust:
  - Cross-platform
  - Redesign and provide a better user experience
  - The current code implementation level is not good, need to rewrite

---

Thanks to [JetBrains](https://www.jetbrains.com/) for donating product licenses to help develop this project <a href="https://www.jetbrains.com/"><img src="logo/jetbrains.svg" width="94" align="center" /></a>
