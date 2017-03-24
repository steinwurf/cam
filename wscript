#! /usr/bin/env python
# encoding: utf-8

APPNAME = 'cam'
VERSION = '3.0.0'

import waflib.extras.wurf_options


def options(opt):

    opt.load('wurf_common_tools')


def resolve(ctx):

    import waflib.extras.wurf_dependency_resolve as resolve

    ctx.load('wurf_common_tools')

    ctx.add_dependency(resolve.ResolveVersion(
        name='waf-tools',
        git_repository='github.com/steinwurf/waf-tools.git',
        major=3))

    ctx.add_dependency(resolve.ResolveVersion(
        name='boost',
        git_repository='github.com/steinwurf/boost.git',
        major=2))

    ctx.add_dependency(resolve.ResolveVersion(
        name='meta',
        git_repository='github.com/steinwurf/meta.git',
        major=2))

    ctx.add_dependency(resolve.ResolveVersion(
        name='nalu',
        git_repository='gitlab.com/steinwurf/nalu.git',
        major=2))

    # Internal dependencies
    if ctx.is_toplevel():

        ctx.add_dependency(resolve.ResolveVersion(
            name='endian',
            git_repository='github.com/steinwurf/endian.git',
            major=3))

        ctx.add_dependency(resolve.ResolveVersion(
            name='gtest',
            git_repository='github.com/steinwurf/gtest.git',
            major=3))


def configure(conf):

    conf.load("wurf_common_tools")

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

    bld.load("wurf_common_tools")

    bld.env.append_unique(
        'DEFINES_STEINWURF_VERSION',
        'STEINWURF_CAM_VERSION="{}"'.format(
            VERSION))

    bld(export_includes=['src'],
        name='cam_includes',
        use=['meta_includes', 'boost_system', 'nalu_includes', 'UDEV', 'USB-1.0'])

    if bld.is_toplevel():

        # Only build tests when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse('test')
        bld.recurse('examples/write_file')
        bld.recurse('examples/uvc_test')
        bld.recurse('examples/canon')
