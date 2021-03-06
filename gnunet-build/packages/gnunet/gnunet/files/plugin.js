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
    var lib = UTF8ToString(library_name);
    var handle = ccallFunc(_dlopen, 'number',
      ['string', 'number'],
      [lib + '.js', 0]);
    if (0 == handle) {
      return 0;
    }
    var sym = ccallFunc(_dlsym, 'number',
      ['number', 'string'],
      [handle, lib + '_init']);
    if (!sym) {
      _dlclose(handle);
      return 0;
    }
    var ret = dynCall('ii', sym, [arg]);
    if (0 == ret) {
      _dlclose(handle);
      return 0;
    }
    return ret;
  },
  GNUNET_PLUGIN_load_all__deps: ['$FS', 'GNUNET_PLUGIN_load'],
  GNUNET_PLUGIN_load_all: function(basename, arg, cb, cb_cls) {
    var prefix = UTF8ToString(basename);
    var entries = [];
    try {
      entries = FS.readdir('.');
    } catch (e) {
      return;
    }
    entries.map(function(entry) {
      if (entry.lastIndexOf(prefix, 0) !== 0)
        return;
      // Remove .js suffix
      entry = entry.substr(0, entry.length - 3);
      var rc = ccallFunc(_GNUNET_PLUGIN_load, 'number',
          ['string', 'number'],
          [entry, arg]);
      if (rc !== 0)
        ccallFunc(getFuncWrapper(cb, 'viii'), 'void',
            ['number', 'string', 'number'],
            [cb_cls, entry, rc]);
    });
  }
});

// vim: set expandtab ts=2 sw=2:
