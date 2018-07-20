import sys, os

from waflib.Tools.compiler_c import c_compiler
from waflib.Tools.compiler_cxx import cxx_compiler

sys.path += [ 'backend/tools/waf-plugins' ]

def options(opt):
  opt.load('defaults')
  opt.load('compiler_c')
  opt.load('compiler_cxx')

def configure(conf):
  conf.load('defaults')
  conf.load('compiler_c')
  conf.load('compiler_cxx')
  conf.env.INCLUDES += [ 'backend/src', 'src' ]
  conf.env.INCLUDES += [ 'external/common']
  conf.env.INCLUDES += [ 'external/common/include', 'include' ]
  conf.env.CXXFLAGS += [ '-g' ]

from waflib.Build import BuildContext
class init_test_class(BuildContext):
  cmd = "init_test"
class init_test_class(BuildContext):
  cmd = "tools"
class simtrade_class(BuildContext):
  cmd = "simtrade"

from lint import add_lint_ignore
def build(bld):
  add_lint_ignore('external')
  add_lint_ignore('backend')
  #add_lint_ignore('src/simtrade/common')
  bld.read_shlib('zbackend', [ 'external/common/lib' ])
  if bld.cmd == "init_test":
    run_init_test(bld)
    return
  if bld.cmd == "tools":
    run_tools(bld)
    return
  if bld.cmd == "simtrade":
    run_simtrade(bld)
    return
  if bld.cmd == "fake_sim":
    run_fake_sim(bld)
    return
  else:
    print "error! " + str(bld.cmd)
    return

def run_init_test(bld):
  bld.read_shlib('zmq', paths=['external/init_test/lib'])
  bld.program(
    target = 'bin/init_test',
    source = 'src/init_test.cpp',
    use = 'zmq',
    includes = ['external/init_test/include']
  )

def run_tools(bld):
  bld.read_shlib('md5', paths=['external/md5str/lib'])
  bld.read_shlib('zbackend', paths=['external/common/lib'])
  bld.program(
    target = 'bin/md5str',
    source = 'src/tools/md5str/main.cpp',
    use = 'md5',
    includes = ['external/md5str/include']
  )

def run_simtrade(bld):
  bld.read_shlib('sodium', paths=['external/zeromq/'])
  bld.read_shlib('zmq', paths=['external/zeromq/'])
  bld.read_shlib('thostmduserapi', paths=['external/ctp/lib'])
  bld.read_shlib('thosttraderapi', paths=['external/ctp/lib'])
  bld.program(
    target = 'bin/strat',
    source = ['src/simtrade/strat/main.cpp',
              'src/simtrade/strat/strategy.cpp'],
    use = 'zmq'
  )
  bld.program(
    target = 'bin/simdata',
    source = ['src/simtrade/simdata/main.cpp',
              'src/simtrade/simdata/datagener.cpp'],
    use = 'zmq'
  )
  bld.program(
    target = 'bin/simorder',
    source = ['src/simtrade/simorder/main.cpp',
              'src/simtrade/simorder/orderlistener.cpp'],
    use = 'zmq'
  )
 
  bld.program(
    target = 'bin/ctpdata',
    source = ['src/simtrade/ctpdata/ctpdata.cpp'],
    includes = ['external/ctp/include'],
    use = 'zmq thostmduserapi'
  )
 
  bld.program(
    target = 'bin/ctporder',
    source = ['src/simtrade/ctporder/main.cpp',
              'src/simtrade/ctporder/listener.cpp',
              'src/simtrade/ctporder/message_sender.cpp'],
    includes = ['external/ctp/include'],
    use = 'zmq thosttraderapi'
  )
