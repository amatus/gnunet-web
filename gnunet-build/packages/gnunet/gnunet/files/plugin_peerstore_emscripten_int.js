// plugin_peerstore_emscripten_int.js - js functions for peerstore plugin
// Copyright (C) 2016  David Barksdale <amatus@amat.us>
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
  peerstore_emscripten_expire_records_int: function(now, cont, cont_cls) {
    var count = 0;
    var store =
     self.psdb.transaction(['peerstore'], 'readwrite').objectStore('peerstore');
    var request = store.index('by_expiry').openCursor(
        IDBKeyRange.upperBound(now, true));
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        count++;
        cursor.delete().onerror = function(e) {
          Module.print('expiry request failed');
        };
        cursor.continue();
      } else {
        if (cont) {
          Runtime.dynCall('vii', cont, [cont_cls, count]);
        }
      }
    };
    request.onerror = function(e) {
      Module.print('cursor request failed');
      Runtime.dynCall('vii', cont, [cont_cls, -1]);
    };
  }, 
  peerstore_emscripten_iterate_records_int: function(sub_system_pointer,
                                                peer_pointer, key_pointer,
                                                iter, iter_cls, wrapper) {
    var sub_system = Pointer_stringify(sub_system_pointer);
    var peer = peer_pointer ? Array.prototype.slice.call(
        HEAP8.subarray(peer_pointer, peer_pointer + 32))
      : null;
    var key = key_pointer ? Pointer_stringify(key_pointer) : null;
    var key_range = [sub_system];
    if (peer) {
      key_range.push(peer);
    }
    if (key) {
      key_range.push(key);
    }
    var store =
      self.psdb.transaction(['peerstore'], 'readonly').objectStore('peerstore');
    var index;
    if (!peer && !key) {
      index = store.index('by_subsystem');
    } else if (!peer) {
      index = store.index('by_key');
    } else if (!key) {
      index = store.index('by_peer');
    } else {
      index = store.index('by_all');
    }
    var request = index.openCursor(IDBKeyRange.only(key_range));
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var stack = Runtime.stackSave();
        var expiry = Runtime.stackAlloc(Runtime.getNativeTypeSize("double"));
        setValue(expiry, cursor.value.expiry, "double");
        ccallFunc(
            Runtime.getFuncWrapper(wrapper, "viiiiiiii"),
            "void",
            ["number", "number", "string", "array", "string", "array", "number",
             "number"],
            [iter, iter_cls, cursor.value.subsystem, cursor.value.peer,
             cursor.value.key, cursor.value.value, cursor.value.value.length,
             expiry]);
        Runtime.stackRestore(stack);
        cursor.continue();
      } else {
        Runtime.dynCall('iiii', iter, [iter_cls, 0, 0]);
      }
    };
    request.onerror = function(e) {
      Module.print('cursor request failed');
      Runtime.dynCall('iiii', iter, [iter_cls, 0, -1]);
    };
  },
  peerstore_emscripten_store_record_int: function(sub_system_pointer,
                                             peer_pointer, key_pointer,
                                             value_pointer, size, expiry,
                                             options, cont, cont_cls) {
    var sub_system = Pointer_stringify(sub_system_pointer);
    var peer = Array.prototype.slice.call(HEAP8.subarray(peer_pointer,
          peer_pointer + 32));
    var key = Pointer_stringify(key_pointer);
    var value = new Uint8Array(HEAP8.subarray(value_pointer,
          value_pointer + size));
    var store =
     self.psdb.transaction(['peerstore'], 'readwrite').objectStore('peerstore');
    var put = function() {
      var request = store.put(
          {sub_system: sub_system,
           peer: peer,
           key: key,
           value: value,
           expiry: expiry});
      request.onerror = function(e) {
        Module.print('put request failed');
        Runtime.dynCall('vii', cont, [cont_cls, -1]);
      };
      request.onsuccess = function(e) {
        Runtime.dynCall('vii', cont, [cont_cls, 1]);
      };
    };
    if (options == 1) {
      var request = store.index('by_all').openCursor(
          IDBKeyRange.only([sub_system, peer, key]));
      request.onsuccess = function(e) {
        var cursor = e.target.result;
        if (cursor) {
          cursor.delete().onerror = function(e) {
            Module.print('replace request failed');
          };
          cursor.continue();
        } else {
          put();
        }
      };
      request.onerror = function(e) {
        Module.print('cursor request failed');
        Runtime.dynCall('vii', cont, [cont_cls, -1]);
      };
    } else {
      put();
    }
  }
});

/* vim: set expandtab ts=2 sw=2: */
