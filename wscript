#! /usr/bin/env python
# encoding: utf-8

APPNAME = 'cam'
VERSION = '6.0.4'

def configure(conf):

    if conf.is_mkspec_platform('linux'):

        errmsg = """not found, is available in the following packages:

            Debian/Ubuntu: apt-get install libudev-dev
            Fedora/CentOS: yum install systemd-devel
        """

        conf.check_cxx(header_name='libudev.h', errmsg=errmsg)

        if not conf.env['LIB_UDEV']:
            conf.check_cxx(lib='udev')

        errmsg = """not found, is available in the following packages:

            Debian/Ubuntu: apt-get install libv4l-dev
        """
        conf.check_cxx(header_name='linux/videodev2.h', errmsg=errmsg)

        errmsg = """not found, is available in the following packages:

            Debian/Ubuntu: apt-get install libusb-1.0-0-dev
        """
        conf.check_cxx(header_name='libusb-1.0/libusb.h', errmsg=errmsg)

        if not conf.env['LIB_USB-1.0']:
            conf.check_cxx(lib='usb-1.0')


def build(bld):

    bld.env.append_unique(
        'DEFINES_STEINWURF_VERSION',
        'STEINWURF_CAM_VERSION="{}"'.format(VERSION))

    bld.stlib(features='cxx',
              source=bld.path.ant_glob('src/**/*.cpp'),
              target='cam',
              use=['meta_includes',
              'boost_system',
              'nalu_includes',
              'platform_includes',
              'UDEV',
              'USB-1.0'],
              export_includes=['src'])

    if bld.is_toplevel():

        # Only build tests when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse('test')
        bld.recurse('examples/write_file')
        bld.recurse('examples/uvc_test')
        bld.recurse('examples/canon')
