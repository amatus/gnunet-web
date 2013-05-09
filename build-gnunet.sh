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
./autogen.sh ||
  die "Uanble to autogen libgcrypt"
emconfigure ./configure --prefix="$SYSROOT" \
  --disable-asm \
  --disable-avx-support \
  --with-gpg-error-prefix="$SYSROOT" \
  ac_cv_func_syslog=no ||
  die "Unable to emconfigure libgcrypt"
emmake make ||
  die "Unable to emmake libgcrypt"
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
./bootstrap
EMCONFIGURE_JS=1 emconfigure ./configure --prefix="$SYSROOT" \
  --with-libgcrypt-prefix="$SYSROOT"
popd

# vim: set expandtab ts=2 sw=2:
