#! /bin/sh
autoreconf -v --install || exit 1
chmod +x ./configure
./configure --enable-maintainer-mode "$@"
