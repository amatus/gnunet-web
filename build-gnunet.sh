#!/bin/sh

die() {
  printf "%s\n" "$1"
  exit 1
}

SYSROOT=$(pwd)/gnunet/sysroot
mkdir -p "$SYSROOT"

# Build libgpg-error
LIBGPG_ERROR_TBZ2=libgpg-error-1.11.tar.bz2
LIBGPG_ERROR_SRCDIR=libgpg-error-1.11
LIBGPG_ERROR_URL=ftp://ftp.gnupg.org/gcrypt/libgpg-error/$LIBGPG_ERROR_TBZ2
if ! [ -f "downloads/$LIBGPG_ERROR_TBZ2" ]; then
  wget -P downloads "$LIBGPG_ERROR_URL" ||
    die "Unable to download $LIBGPG_ERROR_TBZ2"
fi

pushd gnunet
tar -jxf "../downloads/$LIBGPG_ERROR_TBZ2" ||
  die "Unable to extract $LIBGPG_ERROR_TBZ2"
cd "$LIBGPG_ERROR_SRCDIR"
patch -p1 < ../../patches/libgpg-error-1.11.patch
# Build libgpg-error first using build system tools to produce generated
# header files.
./configure ||
  die "Unable to configure libgpg-error"
make ||
  die "Unable to make libgpg-error"
# Build again with emscripten, keeping generated files fresh so make won't
# rebuild them.
emconfigure ./configure --prefix="$SYSROOT" ||
  die "Unable to emconfigure libgpg-error"
touch src/mkerrcodes.h
touch src/mkerrcodes
touch src/code-from-errno.h
touch src/gpg-error.def
emmake make ||
  die "Unable to emmake libgpg-error"
EMMAKEN_JUST_CONFIGURE=true EMCONFIGURE_JS=true emmake make check ||
  die "libgpg-error tests failed"
emmake make install ||
  die "Unable to install libgpg-error"
popd

# Build libgcrypt
LIBGCRYPT_URL=git://git.gnupg.org/libgcrypt.git
if ! [ -d "downloads/libgcrypt" ]; then
  git clone "$LIBGCRYPT_URL" downloads/libgcrypt ||
    die "Unable to clone libgcrypt git repository"
fi

pushd gnunet
git clone ../downloads/libgcrypt libgcrypt ||
  die "Unable to clone libgcrypt git repository"
cd libgcrypt
patch -p1 < ../../patches/libgcrypt.patch
automake --add-missing
./autogen.sh ||
  die "Uanble to autogen libgcrypt"
emconfigure ./configure --enable-maintainer-mode \
  --prefix="$SYSROOT" \
  --disable-asm \
  --disable-avx-support \
  --disable-avx2-support \
  --disable-threads \
  --with-gpg-error-prefix="$SYSROOT" \
  ac_cv_func_syslog=no \
  ac_cv_func_mlock=no \
  gnupg_cv_mlock_is_in_sys_mman=no ||
  die "Unable to emconfigure libgcrypt"
emmake make SUBDIRS="compat mpi cipher random src" \
  LDFLAGS=-Wc,--ignore-dynamic-linking ||
  die "Unable to emmake libgcrypt"
#emmake make SUBDIRS="tests" ||
#  die "Unable to emmake tests"
#touch tests/*.o
#EMMAKEN_JUST_CONFIGURE=true EMCONFIGURE_JS=true emmake make check \
#  SUBDIRS="tests" \
#  LDFLAGS=-Wc,-s,TOTAL_MEMORY=33554432 ||
#  die "Unable to emmake check"
emmake make install SUBDIRS="compat mpi cipher random src" ||
  die "Unable to install libgcrypt"
popd

# Build libunistring
LIBUNISTRING_TGZ=libunistring-0.9.3.tar.gz
LIBUNISTRING_SRCDIR=libunistring-0.9.3
LIBUNISTRING_URL=http://ftp.gnu.org/gnu/libunistring/$LIBUNISTRING_TGZ
if ! [ -f "downloads/$LIBUNISTRING_TGZ" ]; then
  wget -P downloads "$LIBUNISTRING_URL" ||
    die "Unable to download $LIBUNISTRING_TGZ"
fi

pushd gnunet
tar -zxf "../downloads/$LIBUNISTRING_TGZ" ||
  die "Unable to extract $LIBUNISTRING_TGZ"
cd "$LIBUNISTRING_SRCDIR"
patch -p1 < ../../patches/libunistring.patch
emconfigure ./configure --prefix="$SYSROOT" \
  --disable-threads \
  ac_cv_func_uselocale=no \
  am_cv_func_iconv=no ||
  die "Unable to emconfigure libunistring"
emmake make ||
  die "Unable to emmake libunistring"
emmake make install ||
  die "Unable to install libunistring"
popd

# Build zlib
ZLIB_TGZ=zlib-1.2.8.tar.gz
ZLIB_SRCDIR=zlib-1.2.8
ZLIB_URL=http://zlib.net/$ZLIB_TGZ
if ! [ -f "downloads/$ZLIB_TGZ" ]; then
  wget -P downloads "$ZLIB_URL" ||
    die "Unable to download $ZLIB_TGZ"
fi

pushd gnunet
tar -zxf "../downloads/$ZLIB_TGZ" ||
  die "Unable to extract $ZLIB_TGZ"
cd "$ZLIB_SRCDIR"
emconfigure ./configure --prefix="$SYSROOT" ||
  die "Unable to emconfigure zlib"
emmake make ||
  die "Unable to emmake zlib"
emmake make install ||
  die "Unable to install zlib"
popd

# Build libidn
LIBIDN_TGZ=libidn-1.27.tar.gz
LIBIDN_SRCDIR=libidn-1.27
LIBIDN_URL=http://ftp.gnu.org/gnu/libidn/$LIBIDN_TGZ
if ! [ -f "downloads/$LIBIDN_TGZ" ]; then
  wget -P downloads "$LIBIDN_URL" ||
    die "Unable to download $LIBIDN_TGZ"
fi

pushd gnunet
tar -zxf "../downloads/$LIBIDN_TGZ" ||
  die "Unable to extract $LIBIDN_TGZ"
cd "$LIBIDN_SRCDIR"
patch -p1 < ../../patches/libidn-1.27.patch
emconfigure ./configure --prefix="$SYSROOT" ||
  die "Unable to emconfigure libidn"
emmake make ||
  die "Unable to emmake libidn"
emmake make install ||
  die "Unable to install libidn"
popd

# Build fake libextractor
pushd fake-extractor
emmake make ||
  die "Unable to make fake libextractor"
emmake make install DESTDIR="$SYSROOT" ||
  die "Unable to install fake libextractor"
popd

# Build GNUnet
GNUNET_URL=https://gnunet.org/svn/gnunet
if ! [ -d "downloads/gnunet" ]; then
  svn co "$GNUNET_URL" downloads/gnunet ||
    die "Unable to checkout GNUnet svn repository"
fi

pushd gnunet
cp -r ../downloads/gnunet gnunet ||
  die "Unable to copy GNUnet repository"
cd gnunet
patch -p0 < ../../patches/gnunet.patch
./bootstrap ||
  die "Unable to bootstrap GNUnet"
EMCONFIGURE_JS=1 emconfigure ./configure --prefix="$SYSROOT" \
  --with-libgcrypt-prefix="$SYSROOT" \
  --with-libunistring-prefix="$SYSROOT" \
  --with-zlib="$SYSROOT" \
  --with-extractor="$SYSROOT" \
  --with-included-ltdl \
  --with-libidn="$SYSROOT" \
  --without-libcurl \
  --disable-testing \
  ac_cv_lib_idn_stringprep_check_version=yes ||
  die "Unable to configure GNUnet"
emmake make \
  LDFLAGS=-Wc,--ignore-dynamic-linking ||
  die "Unable to make GNUnet"
# Test-build an html file for gnunet-service-peerinfo
ln -s ../../embedded-files/share share
./libtool --tag=CC --mode=link \
  emcc -fno-strict-aliasing -Wall "-I$SYSROOT/include" "-L$SYSROOT/lib" \
  -o src/peerinfo/gnunet-service-peerinfo.html \
  src/peerinfo/gnunet-service-peerinfo.o \
  src/hello/libgnunethello.la \
  src/statistics/libgnunetstatistics.la \
  src/util/libgnunetutil.la \
  "$SYSROOT/lib/libgcrypt.la" \
  "$SYSROOT/lib/libgpg-error.la" \
  -lm -lsocket \
  --js-library ../../service.js \
  --js-library ../../scheduler.js \
  --pre-js ../../pre.js \
  --embed-file share/gnunet/config.d/gnunet.conf
popd

# vim: set expandtab ts=2 sw=2:
