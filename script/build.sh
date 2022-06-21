#!/bin/bash

set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  curl -fsSL https://github.com/KaiserLancelot/klib/releases/download/v1.25.1/klib-1.25.1-Linux.deb \
    -o klib.deb
  sudo dpkg -i klib.deb
else
  echo "The system does not support: $OSTYPE"
  exit 1
fi
