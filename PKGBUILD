pkgname=flagparser
pkgver=0.1.0
pkgrel=3
pkgdesc="Flag parser"
arch=('x86_64')
license=('MIT')
depends=('glibc')
makedepends=('clang' 'make' 'git')

source=("https://github.com/Helix-ISA/flagparser/archive/refs/heads/master.tar.gz")
sha256sums=('SKIP')

build() {
    cd "$srcdir/flagparser-master"
    make
}

package() {
    cd "$srcdir/flagparser-master"

    install -Dm755 bin/flagparser.so \
        "$pkgdir/usr/lib/libflagparser.so"

    install -Dm644 include/flagparser/flagparser.h \
        "$pkgdir/usr/include/flagparser/flagparser.h"

    install -Dm644 include/types.h \
        "$pkgdir/usr/include/flagparser/types.h"
}
