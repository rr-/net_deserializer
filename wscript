# vi: ft=python
from waflib import Logs
import os
from collections import OrderedDict

APPNAME = 'net_deserializer'
try:
    VERSION = os.popen('git describe --tags').read().strip()
    VERSION_LONG = os.popen('git describe --always --dirty --long --tags').read().strip()
except:
    VERSION = '0.0'
    VERSION_LONG = '?'

def options(ctx):
    ctx.load('compiler_cxx')

    ctx.add_option(
        '-d',
        '--debug',
        dest = 'debug',
        default = False,
        action = 'store_true',
        help = 'enable emitting debug information')

def configure(ctx):
    ctx.env.CXXFLAGS = [
        '-Wall',
        '-Wextra',
        '-pedantic',
        '-Wwrite-strings',
        '-Wno-unused-variable',
        '-Wno-unused-parameter',
        '-Wold-style-cast',
        '-ffloat-store',
        '-std=c++14']

    if ctx.options.debug:
        ctx.env.CXXFLAGS += ['-ggdb']
        Logs.info('Debug information enabled')
    else:
        ctx.env.CXXFLAGS += ['-s', '-O3']
        Logs.info('Debug information disabled, pass -d to enable')

    ctx.load('compiler_cxx')

    if ctx.env.DEST_OS in ['win32']:
        ctx.env.LINKFLAGS_UNICODE = ['-municode']

def build(bld):
    path_to_src = bld.path.find_node('src').abspath()
    path_to_lib = bld.path.find_node('lib').abspath()

    program_sources = [bld.path.find_node('src/main.cc')]
    lib_sources = bld.path.ant_glob('lib/**/*.cc')

    bld.objects(
        source = lib_sources,
        target = 'lib',
        cxxflags = ['-iquote', path_to_lib],
        includes = ['lib'])

    bld.program(
        source = program_sources,
        target = APPNAME,
        cxxflags = ['-iquote', path_to_src, '-iquote', path_to_lib],
        defines = [ 'VERSION="' + VERSION_LONG + '"' ],
        includes = ['lib', 'src'],
        use = [ 'lib' ])
