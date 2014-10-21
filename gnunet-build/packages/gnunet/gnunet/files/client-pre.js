// client-pre.js - linked into gnunet-web client library
// Copyright (C) 2013,2014  David Barksdale <amatus@amatus.name>
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

gnunet_prerun = function() {
  // Create /dev/random but it doesn't need to do anything
  var id = FS.makedev(1, 8);
  FS.registerDevice(id, {});
  FS.mkdev('/dev/random', id);

  // Create /dev/urandom
  var id = FS.makedev(1, 9);
  FS.registerDevice(id, {
    read: function(stream, buffer, offset, length, pos) {
      var random_bytes = new Uint8Array(length);
      window.crypto.getRandomValues(random_bytes);
      for (var i = 0; i < length; i++) {
        buffer[offset+i] = random_bytes[i];
      }
      return i;
    },
  });
  FS.mkdev('/dev/urandom', id);
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);

// vim: set expandtab ts=2 sw=2:
