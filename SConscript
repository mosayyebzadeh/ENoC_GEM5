# -*- mode:python -*-

# Copyright (c) 2004-2005 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Nathan Binkert

import array
import bisect
import imp
import marshal
import os
import re
import subprocess
import sys
import zlib

from os.path import basename, dirname, exists, isdir, isfile, join as joinpath

import SCons

# This file defines how to build a particular configuration of gem5
# based on variable settings in the 'env' build environment.

Import('*')

# Children need to see the environment
Export('env')

build_env = [(opt, env[opt]) for opt in export_vars]

from m5.util import code_formatter, compareVersions

########################################################################
# Code for adding source files of various types
#
# When specifying a source file of some type, a set of guards can be
# specified for that file.  When get() is used to find the files, if
# get specifies a set of filters, only files that match those filters
# will be accepted (unspecified filters on files are assumed to be
# false).  Current filters are:
#     main -- specifies the gem5 main() function
#     skip_lib -- do not put this file into the gem5 library
#     skip_no_python -- do not put this file into a no_python library
#       as it embeds compiled Python
#     <unittest> -- unit tests use filters based on the unit test name
#
# A parent can now be specified for a source file and default filter
# values will be retrieved recursively from parents (children override
# parents).
#
def guarded_source_iterator(sources, **guards):
    '''Iterate over a set of sources, gated by a set of guards.'''
    for src in sources:
        for flag,value in guards.iteritems():
            # if the flag is found and has a different value, skip
            # this file
            if src.all_guards.get(flag, False) != value:
                break
        else:
            yield src

class SourceMeta(type):
    '''Meta class for source files that keeps track of all files of a
    particular type and has a get function for finding all functions
    of a certain type that match a set of guards'''
    def __init__(cls, name, bases, dict):
        super(SourceMeta, cls).__init__(name, bases, dict)
        cls.all = []

    def get(cls, **guards):
        '''Find all files that match the specified guards.  If a source
        file does not specify a flag, the default is False'''
        for s in guarded_source_iterator(cls.all, **guards):
            yield s

class SourceFile(object):
    '''Base object that encapsulates the notion of a source file.
    This includes, the source node, target node, various manipulations
    of those.  A source file also specifies a set of guards which
    describing which builds the source file applies to.  A parent can
    also be specified to get default guards from'''
    __metaclass__ = SourceMeta
    def __init__(self, source, parent=None, **guards):
        self.guards = guards
        self.parent = parent

        tnode = source
        if not isinstance(source, SCons.Node.FS.File):
            tnode = File(source)

        self.tnode = tnode
        self.snode = tnode.srcnode()

        for base in type(self).__mro__:
            if issubclass(base, SourceFile):
                base.all.append(self)

    @property
    def filename(self):
        return str(self.tnode)

    @property
    def dirname(self):
        return dirname(self.filename)

    @property
    def basename(self):
        return basename(self.filename)

    @property
    def extname(self):
        index = self.basename.rfind('.')
        if index <= 0:
            # dot files aren't extensions
            return self.basename, None

        return self.basename[:index], self.basename[index+1:]

    @property
    def all_guards(self):
        '''find all guards for this object getting default values
        recursively from its parents'''
        guards = {}
        if self.parent:
            guards.update(self.parent.guards)
        guards.update(self.guards)
        return guards

    def __lt__(self, other): return self.filename < other.filename
    def __le__(self, other): return self.filename <= other.filename
    def __gt__(self, other): return self.filename > other.filename
    def __ge__(self, other): return self.filename >= other.filename
    def __eq__(self, other): return self.filename == other.filename
    def __ne__(self, other): return self.filename != other.filename

    @staticmethod
    def done():
        def disabled(cls, name, *ignored):
            raise RuntimeError("Additional SourceFile '%s'" % name,\
                  "declared, but targets deps are already fixed.")
        SourceFile.__init__ = disabled


class Source(SourceFile):
    current_group = None
    source_groups = { None : [] }

    @classmethod
    def set_group(cls, group):
        if not group in Source.source_groups:
            Source.source_groups[group] = []
        Source.current_group = group

    '''Add a c/c++ source file to the build'''
    def __init__(self, source, Werror=True, **guards):
        '''specify the source file, and any guards'''
        super(Source, self).__init__(source, **guards)

        self.Werror = Werror

        Source.source_groups[Source.current_group].append(self)

class PySource(SourceFile):
    '''Add a python source file to the named package'''
    invalid_sym_char = re.compile('[^A-z0-9_]')
    modules = {}
    tnodes = {}
    symnames = {}

    def __init__(self, package, source, **guards):
        '''specify the python package, the source file, and any guards'''
        super(PySource, self).__init__(source, **guards)

        modname,ext = self.extname
        assert ext == 'py'

        if package:
            path = package.split('.')
        else:
            path = []

        modpath = path[:]
        if modname != '__init__':
            modpath += [ modname ]
        modpath = '.'.join(modpath)

        arcpath = path + [ self.basename ]
        abspath = self.snode.abspath
        if not exists(abspath):
            abspath = self.tnode.abspath

        self.package = package
        self.modname = modname
        self.modpath = modpath
        self.arcname = joinpath(*arcpath)
        self.abspath = abspath
        self.compiled = File(self.filename + 'c')
        self.cpp = File(self.filename + '.cc')
        self.symname = PySource.invalid_sym_char.sub('_', modpath)

        PySource.modules[modpath] = self
        PySource.tnodes[self.tnode] = self
        PySource.symnames[self.symname] = self

class SimObject(PySource):
    '''Add a SimObject python file as a python source object and add
    it to a list of sim object modules'''

    fixed = False
    modnames = []

    def __init__(self, source, **guards):
        '''Specify the source file and any guards (automatically in
        the m5.objects package)'''
        super(SimObject, self).__init__('m5.objects', source, **guards)
        if self.fixed:
            raise AttributeError, "Too late to call SimObject now."

        bisect.insort_right(SimObject.modnames, self.modname)

class ProtoBuf(SourceFile):
    '''Add a Protocol Buffer to build'''

    def __init__(self, source, **guards):
        '''Specify the source file, and any guards'''
        super(ProtoBuf, self).__init__(source, **guards)

        # Get the file name and the extension
        modname,ext = self.extname
        assert ext == 'proto'

        # Currently, we stick to generating the C++ headers, so we
        # only need to track the source and header.
        self.cc_file = File(modname + '.pb.cc')
        self.hh_file = File(modname + '.pb.h')

class UnitTest(object):
    '''Create a UnitTest'''

    all = []
    def __init__(self, target, *sources, **kwargs):
        '''Specify the target name and any sources.  Sources that are
        not SourceFiles are evalued with Source().  All files are
        guarded with a guard of the same name as the UnitTest
        target.'''

        srcs = []
        for src in sources:
            if not isinstance(src, SourceFile):
                src = Source(src, skip_lib=True)
            src.guards[target] = True
            srcs.append(src)

        self.sources = srcs
        self.target = target
        self.main = kwargs.get('main', False)
        UnitTest.all.append(self)

# Children should have access
Export('Source')
Export('PySource')
Export('SimObject')
Export('ProtoBuf')
Export('UnitTest')

########################################################################
#
# Debug Flags
#
debug_flags = {}
def DebugFlag(name, desc=None):
    if name in debug_flags:
        raise AttributeError, "Flag %s already specified" % name
    debug_flags[name] = (name, (), desc)

def CompoundFlag(name, flags, desc=None):
    if name in debug_flags:
        raise AttributeError, "Flag %s already specified" % name

    compound = tuple(flags)
    debug_flags[name] = (name, compound, desc)

Export('DebugFlag')
Export('CompoundFlag')

########################################################################
#
# Set some compiler variables
#

# Include file paths are rooted in this directory.  SCons will
# automatically expand '.' to refer to both the source directory and
# the corresponding build directory to pick up generated include
# files.
env.Append(CPPPATH=Dir('.'))

for extra_dir in extras_dir_list:
    env.Append(CPPPATH=Dir(extra_dir))

# Workaround for bug in SCons version > 0.97d20071212
# Scons bug id: 2006 gem5 Bug id: 308
for root, dirs, files in os.walk(base_dir, topdown=True):
    Dir(root[len(base_dir) + 1:])

########################################################################
#
# Walk the tree and execute all SConscripts in subdirectories
#

here = Dir('.').srcnode().abspath
for root, dirs, files in os.walk(base_dir, topdown=True):
    if root == here:
        # we don't want to recurse back into this SConscript
        continue

    if 'SConscript' in files:
        build_dir = joinpath(env['BUILDDIR'], root[len(base_dir) + 1:])
        Source.set_group(build_dir)
        SConscript(joinpath(root, 'SConscript'), variant_dir=build_dir)

for extra_dir in extras_dir_list:
    prefix_len = len(dirname(extra_dir)) + 1

    # Also add the corresponding build directory to pick up generated
    # include files.
    env.Append(CPPPATH=Dir(joinpath(env['BUILDDIR'], extra_dir[prefix_len:])))

    for root, dirs, files in os.walk(extra_dir, topdown=True):
        # if build lives in the extras directory, don't walk down it
        if 'build' in dirs:
            dirs.remove('build')

        if 'SConscript' in files:
            build_dir = joinpath(env['BUILDDIR'], root[prefix_len:])
            SConscript(joinpath(root, 'SConscript'), variant_dir=build_dir)

for opt in export_vars:
    env.ConfigFile(opt)

def makeTheISA(source, target, env):
    isas = [ src.get_contents() for src in source ]
    target_isa = env['TARGET_ISA']
    def define(isa):
        return isa.upper() + '_ISA'

    def namespace(isa):
        return isa[0].upper() + isa[1:].lower() + 'ISA'


    code = code_formatter()
    code('''\
#ifndef __CONFIG_THE_ISA_HH__
#define __CONFIG_THE_ISA_HH__

''')

    # create defines for the preprocessing and compile-time determination
    for i,isa in enumerate(isas):
        code('#define $0 $1', define(isa), i + 1)
    code()

    # create an enum for any run-time determination of the ISA, we
    # reuse the same name as the namespaces
    code('enum class Arch {')
    for i,isa in enumerate(isas):
        if i + 1 == len(isas):
            code('  $0 = $1', namespace(isa), define(isa))
        else:
            code('  $0 = $1,', namespace(isa), define(isa))
    code('};')

    code('''

#define THE_ISA ${{define(target_isa)}}
#define TheISA ${{namespace(target_isa)}}
#define THE_ISA_STR "${{target_isa}}"

#endif // __CONFIG_THE_ISA_HH__''')

    code.write(str(target[0]))

env.Command('config/the_isa.hh', map(Value, all_isa_list),
            MakeAction(makeTheISA, Transform("CFG ISA", 0)))

def makeTheGPUISA(source, target, env):
    isas = [ src.get_contents() for src in source ]
    target_gpu_isa = env['TARGET_GPU_ISA']
    def define(isa):
        return isa.upper() + '_ISA'

    def namespace(isa):
        return isa[0].upper() + isa[1:].lower() + 'ISA'


    code = code_formatter()
    code('''\
#ifndef __CONFIG_THE_GPU_ISA_HH__
#define __CONFIG_THE_GPU_ISA_HH__

''')

    # create defines for the preprocessing and compile-time determination
    for i,isa in enumerate(isas):
        code('#define $0 $1', define(isa), i + 1)
    code()

    # create an enum for any run-time determination of the ISA, we
    # reuse the same name as the namespaces
    code('enum class GPUArch {')
    for i,isa in enumerate(isas):
        if i + 1 == len(isas):
            code('  $0 = $1', namespace(isa), define(isa))
        else:
            code('  $0 = $1,', namespace(isa), define(isa))
    code('};')

    code('''

#define THE_GPU_ISA ${{define(target_gpu_isa)}}
#define TheGpuISA ${{namespace(target_gpu_isa)}}
#define THE_GPU_ISA_STR "${{target_gpu_isa}}"

#endif // __CONFIG_THE_GPU_ISA_HH__''')

    code.write(str(target[0]))

env.Command('config/the_gpu_isa.hh', map(Value, all_gpu_isa_list),
            MakeAction(makeTheGPUISA, Transform("CFG ISA", 0)))

########################################################################
#
# Prevent any SimObjects from being added after this point, they
# should all have been added in the SConscripts above
#
SimObject.fixed = True

class DictImporter(object):
    '''This importer takes a dictionary of arbitrary module names that
    map to arbitrary filenames.'''
    def __init__(self, modules):
        self.modules = modules
        self.installed = set()

    def __del__(self):
        self.unload()

    def unload(self):
        import sys
        for module in self.installed:
            del sys.modules[module]
        self.installed = set()

    def find_module(self, fullname, path):
        if fullname == 'm5.defines':
            return self

        if fullname == 'm5.objects':
            return self

        if fullname.startswith('_m5'):
            return None

        source = self.modules.get(fullname, None)
        if source is not None and fullname.startswith('m5.objects'):
            return self

        return None

    def load_module(self, fullname):
        mod = imp.new_module(fullname)
        sys.modules[fullname] = mod
        self.installed.add(fullname)

        mod.__loader__ = self
        if fullname == 'm5.objects':
            mod.__path__ = fullname.split('.')
            return mod

        if fullname == 'm5.defines':
            mod.__dict__['buildEnv'] = m5.util.SmartDict(build_env)
            return mod

        source = self.modules[fullname]
        if source.modname == '__init__':
            mod.__path__ = source.modpath
        mod.__file__ = source.abspath

        exec file(source.abspath, 'r') in mod.__dict__

        return mod

import m5.SimObject
import m5.params
from m5.util import code_formatter

m5.SimObject.clear()
m5.params.clear()

# install the python importer so we can grab stuff from the source
# tree itself.  We can't have SimObjects added after this point or
# else we won't know about them for the rest of the stuff.
importer = DictImporter(PySource.modules)
sys.meta_path[0:0] = [ importer ]

# import all sim objects so we can populate the all_objects list
# make sure that we're working with a list, then let's sort it
for modname in SimObject.modnames:
    exec('from m5.objects import %s' % modname)

# we need to unload all of the currently imported modules so that they
# will be re-imported the next time the sconscript is run
importer.unload()
sys.meta_path.remove(importer)

sim_objects = m5.SimObject.allClasses
all_enums = m5.params.allEnums

for name,obj in sorted(sim_objects.iteritems()):
    for param in obj._params.local.values():
        # load the ptype attribute now because it depends on the
        # current version of SimObject.allClasses, but when scons
        # actually uses the value, all versions of
        # SimObject.allClasses will have been loaded
        param.ptype

########################################################################
#
# calculate extra dependencies
#
module_depends = ["m5", "m5.SimObject", "m5.params"]
depends = [ PySource.modules[dep].snode for dep in module_depends ]
depends.sort(key = lambda x: x.name)

########################################################################
#
# Commands for the basic automatically generated python files
#

# Generate Python file containing a dict specifying the current
# buildEnv flags.
def makeDefinesPyFile(target, source, env):
    build_env = source[0].get_contents()

    code = code_formatter()
    code("""
import _m5.core
import m5.util

buildEnv = m5.util.SmartDict($build_env)

compileDate = _m5.core.compileDate
_globals = globals()
for key,val in _m5.core.__dict__.iteritems():
    if key.startswith('flag_'):
        flag = key[5:]
        _globals[flag] = val
del _globals
""")
    code.write(target[0].abspath)

defines_info = Value(build_env)
# Generate a file with all of the compile options in it
env.Command('python/m5/defines.py', defines_info,
            MakeAction(makeDefinesPyFile, Transform("DEFINES", 0)))
PySource('m5', 'python/m5/defines.py')

# Generate python file containing info about the M5 source code
def makeInfoPyFile(target, source, env):
    code = code_formatter()
    for src in source:
        data = ''.join(file(src.srcnode().abspath, 'r').xreadlines())
        code('$src = ${{repr(data)}}')
    code.write(str(target[0]))

# Generate a file that wraps the basic top level files
env.Command('python/m5/info.py',
            [ '#/COPYING', '#/LICENSE', '#/README', ],
            MakeAction(makeInfoPyFile, Transform("INFO")))
PySource('m5', 'python/m5/info.py')

########################################################################
#
# Create all of the SimObject param headers and enum headers
#

def createSimObjectParamStruct(target, source, env):
    assert len(target) == 1 and len(source) == 1

    name = source[0].get_text_contents()
    obj = sim_objects[name]

    code = code_formatter()
    obj.cxx_param_decl(code)
    code.write(target[0].abspath)

def createSimObjectCxxConfig(is_header):
    def body(target, source, env):
        assert len(target) == 1 and len(source) == 1

        name = str(source[0].get_contents())
        obj = sim_objects[name]

        code = code_formatter()
        obj.cxx_config_param_file(code, is_header)
        code.write(target[0].abspath)
    return body

def createEnumStrings(target, source, env):
    assert len(target) == 1 and len(source) == 2

    name = source[0].get_text_contents()
    use_python = source[1].read()
    obj = all_enums[name]

    code = code_formatter()
    obj.cxx_def(code)
    if use_python:
        obj.pybind_def(code)
    code.write(target[0].abspath)

def createEnumDecls(target, source, env):
    assert len(target) == 1 and len(source) == 1

    name = source[0].get_text_contents()
    obj = all_enums[name]

    code = code_formatter()
    obj.cxx_decl(code)
    code.write(target[0].abspath)

def createSimObjectPyBindWrapper(target, source, env):
    name = source[0].get_text_contents()
    obj = sim_objects[name]

    code = code_formatter()
    obj.pybind_decl(code)
    code.write(target[0].abspath)

# Generate all of the SimObject param C++ struct header files
params_hh_files = []
for name,simobj in sorted(sim_objects.iteritems()):
    py_source = PySource.modules[simobj.__module__]
    extra_deps = [ py_source.tnode ]

    hh_file = File('params/%s.hh' % name)
    params_hh_files.append(hh_file)
    env.Command(hh_file, Value(name),
                MakeAction(createSimObjectParamStruct, Transform("SO PARAM")))
    env.Depends(hh_file, depends + extra_deps)

# C++ parameter description files
if GetOption('with_cxx_config'):
    for name,simobj in sorted(sim_objects.iteritems()):
        py_source = PySource.modules[simobj.__module__]
        extra_deps = [ py_source.tnode ]

        cxx_config_hh_file = File('cxx_config/%s.hh' % name)
        cxx_config_cc_file = File('cxx_config/%s.cc' % name)
        env.Command(cxx_config_hh_file, Value(name),
                    MakeAction(createSimObjectCxxConfig(True),
                    Transform("CXXCPRHH")))
        env.Command(cxx_config_cc_file, Value(name),
                    MakeAction(createSimObjectCxxConfig(False),
                    Transform("CXXCPRCC")))
        env.Depends(cxx_config_hh_file, depends + extra_deps +
                    [File('params/%s.hh' % name), File('sim/cxx_config.hh')])
        env.Depends(cxx_config_cc_file, depends + extra_deps +
                    [cxx_config_hh_file])
        Source(cxx_config_cc_file)

    cxx_config_init_cc_file = File('cxx_config/init.cc')

    def createCxxConfigInitCC(target, source, env):
        assert len(target) == 1 and len(source) == 1

        code = code_formatter()

        for name,simobj in sorted(sim_objects.iteritems()):
            if not hasattr(simobj, 'abstract') or not simobj.abstract:
                code('#include "cxx_config/${name}.hh"')
        code()
        code('void cxxConfigInit()')
        code('{')
        code.indent()
        for name,simobj in sorted(sim_objects.iteritems()):
            not_abstract = not hasattr(simobj, 'abstract') or \
                not simobj.abstract
            if not_abstract and 'type' in simobj.__dict__:
                code('cxx_config_directory["${name}"] = '
                     '${name}CxxConfigParams::makeDirectoryEntry();')
        code.dedent()
        code('}')
        code.write(target[0].abspath)

    py_source = PySource.modules[simobj.__module__]
    extra_deps = [ py_source.tnode ]
    env.Command(cxx_config_init_cc_file, Value(name),
        MakeAction(createCxxConfigInitCC, Transform("CXXCINIT")))
    cxx_param_hh_files = ["cxx_config/%s.hh" % simobj
        for name,simobj in sorted(sim_objects.iteritems())
        if not hasattr(simobj, 'abstract') or not simobj.abstract]
    Depends(cxx_config_init_cc_file, cxx_param_hh_files +
            [File('sim/cxx_config.hh')])
    Source(cxx_config_init_cc_file)

# Generate all enum header files
for name,enum in sorted(all_enums.iteritems()):
    py_source = PySource.modules[enum.__module__]
    extra_deps = [ py_source.tnode ]

    cc_file = File('enums/%s.cc' % name)
    env.Command(cc_file, [Value(name), Value(env['USE_PYTHON'])],
                MakeAction(createEnumStrings, Transform("ENUM STR")))
    env.Depends(cc_file, depends + extra_deps)
    Source(cc_file)

    hh_file = File('enums/%s.hh' % name)
    env.Command(hh_file, Value(name),
                MakeAction(createEnumDecls, Transform("ENUMDECL")))
    env.Depends(hh_file, depends + extra_deps)

# Generate SimObject Python bindings wrapper files
if env['USE_PYTHON']:
    for name,simobj in sorted(sim_objects.iteritems()):
        py_source = PySource.modules[simobj.__module__]
        extra_deps = [ py_source.tnode ]
        cc_file = File('python/_m5/param_%s.cc' % name)
        env.Command(cc_file, Value(name),
                    MakeAction(createSimObjectPyBindWrapper,
                               Transform("SO PyBind")))
        env.Depends(cc_file, depends + extra_deps)
        Source(cc_file)

# Build all protocol buffers if we have got protoc and protobuf available
if env['HAVE_PROTOBUF']:
    for proto in ProtoBuf.all:
        # Use both the source and header as the target, and the .proto
        # file as the source. When executing the protoc compiler, also
        # specify the proto_path to avoid having the generated files
        # include the path.
        env.Command([proto.cc_file, proto.hh_file], proto.tnode,
                    MakeAction('$PROTOC --cpp_out ${TARGET.dir} '
                               '--proto_path ${SOURCE.dir} $SOURCE',
                               Transform("PROTOC")))

        # Add the C++ source file
        Source(proto.cc_file, **proto.guards)
elif ProtoBuf.all:
    print 'Got protobuf to build, but lacks support!'
    Exit(1)

#
# Handle debug flags
#
def makeDebugFlagCC(target, source, env):
    assert(len(target) == 1 and len(source) == 1)

    code = code_formatter()

    # delay definition of CompoundFlags until after all the definition
    # of all constituent SimpleFlags
    comp_code = code_formatter()

    # file header
    code('''
/*
 * DO NOT EDIT THIS FILE! Automatically generated by SCons.
 */

#include "base/debug.hh"

namespace Debug {

''')

    for name, flag in sorted(source[0].read().iteritems()):
        n, compound, desc = flag
        assert n == name

        if not compound:
            code('SimpleFlag $name("$name", "$desc");')
        else:
            comp_code('CompoundFlag $name("$name", "$desc",')
            comp_code.indent()
            last = len(compound) - 1
            for i,flag in enumerate(compound):
                if i != last:
                    comp_code('&$flag,')
                else:
                    comp_code('&$flag);')
            comp_code.dedent()

    code.append(comp_code)
    code()
    code('} // namespace Debug')

    code.write(str(target[0]))

def makeDebugFlagHH(target, source, env):
    assert(len(target) == 1 and len(source) == 1)

    val = eval(source[0].get_contents())
    name, compound, desc = val

    code = code_formatter()

    # file header boilerplate
    code('''\
/*
 * DO NOT EDIT THIS FILE! Automatically generated by SCons.
 */

#ifndef __DEBUG_${name}_HH__
#define __DEBUG_${name}_HH__

namespace Debug {
''')

    if compound:
        code('class CompoundFlag;')
    code('class SimpleFlag;')

    if compound:
        code('extern CompoundFlag $name;')
        for flag in compound:
            code('extern SimpleFlag $flag;')
    else:
        code('extern SimpleFlag $name;')

    code('''
}

#endif // __DEBUG_${name}_HH__
''')

    code.write(str(target[0]))

for name,flag in sorted(debug_flags.iteritems()):
    n, compound, desc = flag
    assert n == name

    hh_file = 'debug/%s.hh' % name
    env.Command(hh_file, Value(flag),
                MakeAction(makeDebugFlagHH, Transform("TRACING", 0)))

env.Command('debug/flags.cc', Value(debug_flags),
            MakeAction(makeDebugFlagCC, Transform("TRACING", 0)))
Source('debug/flags.cc')

# version tags
tags = \
env.Command('sim/tags.cc', None,
            MakeAction('util/cpt_upgrader.py --get-cc-file > $TARGET',
                       Transform("VER TAGS")))
env.AlwaysBuild(tags)

# Embed python files.  All .py files that have been indicated by a
# PySource() call in a SConscript need to be embedded into the M5
# library.  To do that, we compile the file to byte code, marshal the
# byte code, compress it, and then generate a c++ file that
# inserts the result into an array.
def embedPyFile(target, source, env):
    def c_str(string):
        if string is None:
            return "0"
        return '"%s"' % string

    '''Action function to compile a .py into a code object, marshal
    it, compress it, and stick it into an asm file so the code appears
    as just bytes with a label in the data section'''

    src = file(str(source[0]), 'r').read()

    pysource = PySource.tnodes[source[0]]
    compiled = compile(src, pysource.abspath, 'exec')
    marshalled = marshal.dumps(compiled)
    compressed = zlib.compress(marshalled)
    data = compressed
    sym = pysource.symname

    code = code_formatter()
    code('''\
#include "sim/init.hh"

namespace {

const uint8_t data_${sym}[] = {
''')
    code.indent()
    step = 16
    for i in xrange(0, len(data), step):
        x = array.array('B', data[i:i+step])
        code(''.join('%d,' % d for d in x))
    code.dedent()

    code('''};

EmbeddedPython embedded_${sym}(
    ${{c_str(pysource.arcname)}},
    ${{c_str(pysource.abspath)}},
    ${{c_str(pysource.modpath)}},
    data_${sym},
    ${{len(data)}},
    ${{len(marshalled)}});

} // anonymous namespace
''')
    code.write(str(target[0]))

for source in PySource.all:
    env.Command(source.cpp, source.tnode,
                MakeAction(embedPyFile, Transform("EMBED PY")))
    Source(source.cpp, skip_no_python=True)

########################################################################
#
# Define binaries.  Each different build type (debug, opt, etc.) gets
# a slightly different build environment.
#

# List of constructed environments to pass back to SConstruct
date_source = Source('base/date.cc', skip_lib=True)

# Capture this directory for the closure makeEnv, otherwise when it is
# called, it won't know what directory it should use.
variant_dir = Dir('.').path
def variant(*path):
    return os.path.join(variant_dir, *path)
def variantd(*path):
    return variant(*path)+'/'

# Function to create a new build environment as clone of current
# environment 'env' with modified object suffix and optional stripped
# binary.  Additional keyword arguments are appended to corresponding
# build environment vars.
def makeEnv(env, label, objsfx, strip=False, disable_partial=False, **kwargs):
    # SCons doesn't know to append a library suffix when there is a '.' in the
    # name.  Use '_' instead.
    libname = variant('gem5_' + label)
    exename = variant('gem5.' + label)
    secondary_exename = variant('m5.' + label)

    new_env = env.Clone(OBJSUFFIX=objsfx, SHOBJSUFFIX=objsfx + 's')
    new_env.Label = label
    new_env.Append(**kwargs)

    if env['GCC']:
        # The address sanitizer is available for gcc >= 4.8
        if GetOption('with_asan'):
            if GetOption('with_ubsan') and \
                    compareVersions(env['GCC_VERSION'], '4.9') >= 0:
                new_env.Append(CCFLAGS=['-fsanitize=address,undefined',
                                        '-fno-omit-frame-pointer'])
                new_env.Append(LINKFLAGS='-fsanitize=address,undefined')
            else:
                new_env.Append(CCFLAGS=['-fsanitize=address',
                                        '-fno-omit-frame-pointer'])
                new_env.Append(LINKFLAGS='-fsanitize=address')
        # Only gcc >= 4.9 supports UBSan, so check both the version
        # and the command-line option before adding the compiler and
        # linker flags.
        elif GetOption('with_ubsan') and \
                compareVersions(env['GCC_VERSION'], '4.9') >= 0:
            new_env.Append(CCFLAGS='-fsanitize=undefined')
            new_env.Append(LINKFLAGS='-fsanitize=undefined')


    if env['CLANG']:
        # We require clang >= 3.1, so there is no need to check any
        # versions here.
        if GetOption('with_ubsan'):
            if GetOption('with_asan'):
                new_env.Append(CCFLAGS=['-fsanitize=address,undefined',
                                        '-fno-omit-frame-pointer'])
                new_env.Append(LINKFLAGS='-fsanitize=address,undefined')
            else:
                new_env.Append(CCFLAGS='-fsanitize=undefined')
                new_env.Append(LINKFLAGS='-fsanitize=undefined')

        elif GetOption('with_asan'):
            new_env.Append(CCFLAGS=['-fsanitize=address',
                                    '-fno-omit-frame-pointer'])
            new_env.Append(LINKFLAGS='-fsanitize=address')

    werror_env = new_env.Clone()
    # Treat warnings as errors but white list some warnings that we
    # want to allow (e.g., deprecation warnings).
    werror_env.Append(CCFLAGS=['-Werror',
                               '-Wno-error=deprecated-declarations',
                               '-Wno-error=deprecated',
                               ])

    def make_obj(source, static, extra_deps = None):
        '''This function adds the specified source to the correct
        build environment, and returns the corresponding SCons Object
        nodes'''

        if source.Werror:
            env = werror_env
        else:
            env = new_env

        if static:
            obj = env.StaticObject(source.tnode)
        else:
            obj = env.SharedObject(source.tnode)

        if extra_deps:
            env.Depends(obj, extra_deps)

        return obj

    lib_guards = {'main': False, 'skip_lib': False}

    # Without Python, leave out all Python content from the library
    # builds.  The option doesn't affect gem5 built as a program
    if GetOption('without_python'):
        lib_guards['skip_no_python'] = False

    static_objs = []
    shared_objs = []
    for s in guarded_source_iterator(Source.source_groups[None], **lib_guards):
        static_objs.append(make_obj(s, True))
        shared_objs.append(make_obj(s, False))

    partial_objs = []
    for group, all_srcs in Source.source_groups.iteritems():
        # If these are the ungrouped source files, skip them.
        if not group:
            continue

        # Get a list of the source files compatible with the current guards.
        srcs = [ s for s in guarded_source_iterator(all_srcs, **lib_guards) ]
        # If there aren't any left, skip this group.
        if not srcs:
            continue

        # If partial linking is disabled, add these sources to the build
        # directly, and short circuit this loop.
        if disable_partial:
            for s in srcs:
                static_objs.append(make_obj(s, True))
                shared_objs.append(make_obj(s, False))
            continue

        # Set up the static partially linked objects.
        source_objs = [ make_obj(s, True) for s in srcs ]
        file_name = new_env.subst("${OBJPREFIX}lib${OBJSUFFIX}.partial")
        target = File(joinpath(group, file_name))
        partial = env.PartialStatic(target=target, source=source_objs)
        static_objs.append(partial)

        # Set up the shared partially linked objects.
        source_objs = [ make_obj(s, False) for s in srcs ]
        file_name = new_env.subst("${SHOBJPREFIX}lib${SHOBJSUFFIX}.partial")
        target = File(joinpath(group, file_name))
        partial = env.PartialShared(target=target, source=source_objs)
        shared_objs.append(partial)

    static_date = make_obj(date_source, static=True, extra_deps=static_objs)
    static_objs.append(static_date)

    shared_date = make_obj(date_source, static=False, extra_deps=shared_objs)
    shared_objs.append(shared_date)

    # First make a library of everything but main() so other programs can
    # link against m5.
    static_lib = new_env.StaticLibrary(libname, static_objs)
    shared_lib = new_env.SharedLibrary(libname, shared_objs)

    # Now link a stub with main() and the static library.
    main_objs = [ make_obj(s, True) for s in Source.get(main=True) ]

    for test in UnitTest.all:
        flags = { test.target : True }
        test_sources = Source.get(**flags)
        test_objs = [ make_obj(s, static=True) for s in test_sources ]
        if test.main:
            test_objs += main_objs
        path = variant('unittest/%s.%s' % (test.target, label))
        new_env.Program(path, test_objs + static_objs)

    progname = exename
    if strip:
        progname += '.unstripped'

    targets = new_env.Program(progname, main_objs + static_objs)

    if strip:
        if sys.platform == 'sunos5':
            cmd = 'cp $SOURCE $TARGET; strip $TARGET'
        else:
            cmd = 'strip $SOURCE -o $TARGET'
        targets = new_env.Command(exename, progname,
                    MakeAction(cmd, Transform("STRIP")))

    new_env.Command(secondary_exename, exename,
            MakeAction('ln $SOURCE $TARGET', Transform("HARDLINK")))

    new_env.M5Binary = targets[0]

    # Set up regression tests.
    SConscript(os.path.join(env.root.abspath, 'tests', 'SConscript'),
               variant_dir=variantd('tests', new_env.Label),
               exports={ 'env' : new_env }, duplicate=False)

# Start out with the compiler flags common to all compilers,
# i.e. they all use -g for opt and -g -pg for prof
ccflags = {'debug' : [], 'opt' : ['-g'], 'fast' : [], 'prof' : ['-g', '-pg'],
           'perf' : ['-g']}

# Start out with the linker flags common to all linkers, i.e. -pg for
# prof, and -lprofiler for perf. The -lprofile flag is surrounded by
# no-as-needed and as-needed as the binutils linker is too clever and
# simply doesn't link to the library otherwise.
ldflags = {'debug' : [], 'opt' : [], 'fast' : [], 'prof' : ['-pg'],
           'perf' : ['-Wl,--no-as-needed', '-lprofiler', '-Wl,--as-needed']}

# For Link Time Optimization, the optimisation flags used to compile
# individual files are decoupled from those used at link time
# (i.e. you can compile with -O3 and perform LTO with -O0), so we need
# to also update the linker flags based on the target.
if env['GCC']:
    if sys.platform == 'sunos5':
        ccflags['debug'] += ['-gstabs+']
    else:
        ccflags['debug'] += ['-ggdb3']
    ldflags['debug'] += ['-O0']
    # opt, fast, prof and perf all share the same cc flags, also add
    # the optimization to the ldflags as LTO defers the optimization
    # to link time
    for target in ['opt', 'fast', 'prof', 'perf']:
        ccflags[target] += ['-O3']
        ldflags[target] += ['-O3']

    ccflags['fast'] += env['LTO_CCFLAGS']
    ldflags['fast'] += env['LTO_LDFLAGS']
elif env['CLANG']:
    ccflags['debug'] += ['-g', '-O0']
    # opt, fast, prof and perf all share the same cc flags
    for target in ['opt', 'fast', 'prof', 'perf']:
        ccflags[target] += ['-O3']
else:
    print 'Unknown compiler, please fix compiler options'
    Exit(1)


# To speed things up, we only instantiate the build environments we
# need.  We try to identify the needed environment for each target; if
# we can't, we fall back on instantiating all the environments just to
# be safe.
target_types = ['debug', 'opt', 'fast', 'prof', 'perf']
obj2target = {'do': 'debug', 'o': 'opt', 'fo': 'fast', 'po': 'prof',
              'gpo' : 'perf'}

def identifyTarget(t):
    ext = t.split('.')[-1]
    if ext in target_types:
        return ext
    if obj2target.has_key(ext):
        return obj2target[ext]
    match = re.search(r'/tests/([^/]+)/', t)
    if match and match.group(1) in target_types:
        return match.group(1)
    return 'all'

needed_envs = [identifyTarget(target) for target in BUILD_TARGETS]
if 'all' in needed_envs:
    needed_envs += target_types

def makeEnvirons(target, source, env):
    # cause any later Source() calls to be fatal, as a diagnostic.
    Source.done()

    # Debug binary
    if 'debug' in needed_envs:
        makeEnv(env, 'debug', '.do',
                CCFLAGS = Split(ccflags['debug']),
                CPPDEFINES = ['DEBUG', 'TRACING_ON=1'],
                LINKFLAGS = Split(ldflags['debug']))

    # Optimized binary
    if 'opt' in needed_envs:
        makeEnv(env, 'opt', '.o',
                CCFLAGS = Split(ccflags['opt']),
                CPPDEFINES = ['TRACING_ON=1'],
                LINKFLAGS = Split(ldflags['opt']))

    # "Fast" binary
    if 'fast' in needed_envs:
        disable_partial = \
                env.get('BROKEN_INCREMENTAL_LTO', False) and \
                GetOption('force_lto')
        makeEnv(env, 'fast', '.fo', strip = True,
                CCFLAGS = Split(ccflags['fast']),
                CPPDEFINES = ['NDEBUG', 'TRACING_ON=0'],
                LINKFLAGS = Split(ldflags['fast']),
                disable_partial=disable_partial)

    # Profiled binary using gprof
    if 'prof' in needed_envs:
        makeEnv(env, 'prof', '.po',
                CCFLAGS = Split(ccflags['prof']),
                CPPDEFINES = ['NDEBUG', 'TRACING_ON=0'],
                LINKFLAGS = Split(ldflags['prof']))

    # Profiled binary using google-pprof
    if 'perf' in needed_envs:
        makeEnv(env, 'perf', '.gpo',
                CCFLAGS = Split(ccflags['perf']),
                CPPDEFINES = ['NDEBUG', 'TRACING_ON=0'],
                LINKFLAGS = Split(ldflags['perf']))

# The MakeEnvirons Builder defers the full dependency collection until
# after processing the ISA definition (due to dynamically generated
# source files).  Add this dependency to all targets so they will wait
# until the environments are completely set up.  Otherwise, a second
# process (e.g. -j2 or higher) will try to compile the requested target,
# not know how, and fail.
env.Append(BUILDERS = {'MakeEnvirons' :
                        Builder(action=MakeAction(makeEnvirons,
                                                  Transform("ENVIRONS", 1)))})

isa_target = '#${VARIANT_NAME}-deps'
environs = '#${VARIANT_NAME}-environs'
env.Depends('#all-deps', isa_target)
env.Depends('#all-environs', environs)
env.ScanISA(isa_target, File('arch/%s/generated/inc.d' % env['TARGET_ISA']))
envSetup = env.MakeEnvirons(environs, isa_target)

# make sure no -deps targets occur before all ISAs are complete
env.Depends(isa_target, '#all-isas')
# likewise for -environs targets and all the -deps targets
env.Depends(environs, '#all-deps')