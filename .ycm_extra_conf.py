# Here's the license text for this file:
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

import os
import ycm_core
import pkgconfig

flags = [
'-std=gnu++14',
# ...and the same thing goes for the magic -x option which specifies the
# language that the files to be compiled are written in. This is mostly
# relevant for c++ headers.
# For a C project, you would set this to 'c' instead of 'c++'.
'-x',
'c++',
'./ClangCompleter',
'-pthread',
'-DDF_DEBUG=1',
'-DDF_DAEMON_PATH="/fake/path/string"',
'-DDF_VERBOSE=1',
'-DDF_INPUT_PIPE_PATH="/fake/path/string"',
'-DDF_OUTPUT_PIPE_PATH="/fake/path/string"',
'-DDF_LOCK_FILE_PATH="/fake/path/string"',
'-DDF_VERIFY_PATH=1',
'-DDF_VERIFY_PATH_SECURITY=1',
'-DDF_VERIFY_PARENT_PATH_SECURITY=1',
'-DDF_REQUIRE_RUNNING_PARENT=1',
'-DDF_REQUIRED_PARENT_PATH="/fake/path/string"',
'-DDF_TIMEOUT=1',
 ]

pkgConfigFlags = []

for pflag in pkgConfigFlags:
  flagList = pkgconfig.cflags(pflag).split(' ')
  for f in flagList:
    flags.append(f)

for dirname, subdirList, fileList in os.walk('./Include'):
  flags.append('-I'+dirname)

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )

def FlagsForFile(filename, **kwargs):
  return {
    'flags': flags,
    'include_paths_relative_to_dir': DirectoryOfThisScript()
  }
