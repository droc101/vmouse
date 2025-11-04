# Maintainer: droc101 <droc101(at)droc101(dot)dev>

_pkgbase=vmouse
_dkmsname=hid-vmouse
pkgname=vmouse-dkms-git
pkgver=1.0.1
pkgrel=1
pkgdesc="Virtual mouse kernel module with DKMS support"
arch=('x86_64')
url="https://github.com/droc101/vmouse"
license=('GPL')
depends=('dkms')
source=("$url/archive/refs/heads/main.tar.gz")
sha256sums=('SKIP')
install=vmouse.install

package() {
  install -d "${pkgdir}/usr/src/${_pkgbase}-${pkgver}"
  cp -r "$srcdir/vmouse-main/"* "${pkgdir}/usr/src/${_pkgbase}-${pkgver}/"

  # Add modules-load.d entry to autoload module on boot
  install -Dm644 /dev/null "${pkgdir}/etc/modules-load.d/${_dkmsname}.conf"
  echo "${_dkmsname}" >> "${pkgdir}/etc/modules-load.d/${_dkmsname}.conf"
}
