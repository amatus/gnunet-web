// plugin.js - plugin loading for gnunet-web services
// Copyright (C) 2014  David Barksdale <amatus@amatus.name>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

mergeInto(LibraryManager.library, {
  GNUNET_PLUGIN_load__deps: ['dlclose', 'dlsym', 'dlopen'],
  GNUNET_PLUGIN_load: function(library_name, arg) {
    var handle = _dlopen(library_name, 0);
    if (0 == handle) {
      return 0;
    }
    var init_symbol = Pointer_stringify(library_name) + "_init";
    var stack = Runtime.stackSave();
    var str = allocate(intArrayFromString(init_symbol), 'i8', ALLOC_STACK);
    var sym = _dlsym(handle, str);
    Runtime.stackRestore(stack);
    if (!sym) {
      dlclose(handle);
      return 0;
    }
    var ret = Runtime.dynCall('ii', sym, [arg]);
    if (0 == ret) {
      dlclose(handle);
      return 0;
    }
    return ret;
  }
});

// vim: set expandtab ts=2 sw=2:
