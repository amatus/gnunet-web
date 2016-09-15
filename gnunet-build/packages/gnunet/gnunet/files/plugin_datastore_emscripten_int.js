// plugin_datastore_emscripten_int.js - js functions for datastore plugin
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
  emscripten_plugin_put_int: function(key_pointer, data_pointer, size, type,
                                 priority, anonymity, replication, expiry,
                                 vhash_pointer, cont, cont_cls) {
    var key = Array.prototype.slice.call(HEAP8.subarray(key_pointer,
          key_pointer + 64));
    var data = Array.prototype.slice.call(HEAP8.subarray(data_pointer,
          data_pointer + size));
    var vhash = Array.prototype.slice.call(HEAP8.subarray(vhash_pointer,
          vhash_pointer + 64));
    var request = self.dsdb.transaction(['datastore'], 'readwrite')
                      .objectStore('datastore')
                      .put({key: key,
                            data: data,
                            type: type,
                            priority: priority,
                            anonymity: anonymity,
                            replication: replication,
                            expiry: expiry,
                            vhash: vhash});
    request.onerror = function(e) {
      console.error('put request failed');
      Runtime.dynCall('viiiii', cont, [cont_cls, key_pointer, size, -1, 0]);
    };
    request.onsuccess = function(e) {
      Runtime.dynCall('viiiii', cont, [cont_cls, key_pointer, size, 1, 0]);
    };
  },
  emscripten_plugin_get_key_int: function(offset, key_pointer, vhash_pointer,
                                     type, proc, proc_cls,
                                     datum_processor_wrapper) {
    var key = key_pointer ? Array.prototype.slice.call(HEAP8.subarray(
          key_pointer, key_pointer + 64)) : null;
    var vhash = vhash_pointer ? Array.prototype.slice.call(HEAP8.subarray(
          vhash_pointer, vhash_pointer + 64)) : null;
    var count = 0;
    var request = self.dsdb.transaction(['datastore'], 'readwrite')
                      .objectStore('datastore').index('by_key')
                      .openCursor(key);
    request.onerror = function(e) {
      console.error('cursor request failed');
      Runtime.dynCall('iiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        // optional filter by type
        if (type && type != cursor.value.type) {
          cursor.continue();
          return;
        }
        // optional filter by vhash
        if (vhash != null) {
          if (vhash.length != cursor.value.vhash.length
              || !vhash.every(function(x, i) {
                                return x == cursor.value.vhash[i]
                              })) {
            cursor.continue();
            return;
          }
        }
        // filter by offset
        if (offset != 0) {
          --offset;
          ++count;
          cursor.continue();
          return;
        }
        // got a result
        var stack = Runtime.stackSave();
        var expiry = Runtime.stackAlloc(Runtime.getNativeTypeSize('double'));
        setValue(expiry, cursor.value.expiry, 'double');
        var ret = ccallFunc(
            Runtime.getFuncWrapper(datum_processor_wrapper,
              'iiiiiiiiiii'),
            'number',
            ['number', 'number', 'array', 'number', 'array', 'number', 'number',
             'number', 'number', 'number'],
            [proc, proc_cls, cursor.value.key, cursor.value.data.length,
             cursor.value.data, cursor.value.type, cursor.value.priority,
             cursor.value.anonymity, expiry, cursor.value.uid]);
        Runtime.stackRestore(stack);
        if (!ret) {
          cursor.delete().onerror = function(e) {
            console.error('delete request failed');
          }
        }
      } else {
        // did offset wrap around?
        if (count != 0) {
          offset = offset % count;
          // recurse
          _emscripten_plugin_get_key_int(offset, key_pointer, vhash_pointer,
              type, proc, proc_cls, datum_processor_wrapper);
          return;
        }
        // not found
        Runtime.dynCall('iiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_replication_int: function(proc, proc_cls,
      datum_processor_wrapper) {
    var transaction = self.dsdb.transaction(['datastore'], 'readwrite');
    var request = transaction.objectStore('datastore').index('by_replication')
                             .openCursor(null, 'prev');
    request.onerror = function(e) {
      console.error('cursor request failed');
      Runtime.dynCall('iiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        // got a result
        if (cursor.value.replication > 0) {
          --cursor.value.replication;
        }
        var request = transaction.objectStore('datastore').put(value);
        request.onerror = function(e) {
          console.error('put request failed');
          Runtime.dynCall('iiiiiiiiiii', proc,
            [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
        };
        request.onsuccess = function(e) {
          var stack = Runtime.stackSave();
          var expiry = Runtime.stackAlloc(Runtime.getNativeTypeSize('double'));
          setValue(expiry, cursor.value.expiry, 'double');
          var ret = ccallFunc(
              Runtime.getFuncWrapper(datum_processor_wrapper, 'iiiiiiiiiii'),
              'number',
              ['number', 'number', 'array', 'number', 'array', 'number',
               'number', 'number', 'number', 'number'],
              [proc, proc_cls, cursor.value.key, cursor.value.data.length,
               cursor.value.data, cursor.value.type, cursor.value.priority,
               cursor.value.anonymity, expiry, cursor.value.uid]);
          Runtime.stackRestore(stack);
          if (!ret) {
            cursor.delete().onerror = function(e) {
              console.error('delete request failed');
            }
          }
        };
      } else {
        Runtime.dynCall('iiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_expiration_int: function(proc, proc_cls, now,
      datum_processor_wrapper) {
    var request = self.dsdb.transaction(['datastore'], 'readwrite')
                      .objectStore('datastore').index('by_expiry')
                      .openCursor(IDBKeyRange.upperBound(now, true));
    request.onerror = function(e) {
      console.error('cursor request failed');
      Runtime.dynCall('iiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        // got a result
        var stack = Runtime.stackSave();
        var expiry = Runtime.stackAlloc(Runtime.getNativeTypeSize('double'));
        setValue(expiry, cursor.value.expiry, 'double');
        var ret = ccallFunc(
            Runtime.getFuncWrapper(datum_processor_wrapper,
              'iiiiiiiiiii'),
            'number',
            ['number', 'number', 'array', 'number', 'array', 'number', 'number',
             'number', 'number', 'number'],
            [proc, proc_cls, cursor.value.key, cursor.value.data.length,
             cursor.value.data, cursor.value.type, cursor.value.priority,
             cursor.value.anonymity, expiry, cursor.value.uid]);
        Runtime.stackRestore(stack);
        if (!ret) {
          cursor.delete().onerror = function(e) {
            console.error('delete request failed');
          }
        }
      } else {
        Runtime.dynCall('iiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_update_int: function(uid, delta, expiry, cont,
      cont_cls) {
    var transaction = self.dsdb.transaction(['datastore'], 'readwrite');
    var request = transaction.objectStore('datastore').get(uid);
    request.onerror = function(e) {
      console.warning('get request failed');
      Runtime.dynCall('viii', cont, [cont_cls, -1, 0]);
    };
    request.onsuccess = function(e) {
      var value = e.target.result;
      value.priority += delta;
      if (value.priority < 0) {
        value.priority = 0;
      }
      if (value.expiry < expiry) {
        value.expiry = expiry;
      }
      var request = transaction.objectStore('datastore').put(value);
      request.onerror = function(e) {
        console.error('put request failed');
        Runtime.dynCall('viii', cont, [cont_cls, -1, 0]);
      };
      request.onsuccess = function(e) {
        Runtime.dynCall('viii', cont, [cont_cls, 1, 0]);
      };
    };
  }, emscripten_plugin_get_zero_anonymity_int: function(offset, type, proc,
      proc_cls, datum_processor_wrapper) {
    var count = 0;
    var request = self.dsdb.transaction(['datastore'], 'readwrite')
                      .objectStore('datastore').index('by_anon_type')
                      .openCursor([0, type]);
    request.onerror = function(e) {
      console.error('cursor request failed');
      Runtime.dynCall('iiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        // filter by offset
        if (offset != 0) {
          --offset;
          ++count;
          cursor.continue();
          return;
        }
        // got a result
        var stack = Runtime.stackSave();
        var expiry = Runtime.stackAlloc(Runtime.getNativeTypeSize('double'));
        setValue(expiry, cursor.value.expiry, 'double');
        var ret = ccallFunc(
            Runtime.getFuncWrapper(datum_processor_wrapper,
              'iiiiiiiiiii'),
            'number',
            ['number', 'number', 'array', 'number', 'array', 'number', 'number',
             'number', 'number', 'number'],
            [proc, proc_cls, cursor.value.key, cursor.value.data.length,
             cursor.value.data, cursor.value.type, cursor.value.priority,
             cursor.value.anonymity, expiry, cursor.value.uid]);
        Runtime.stackRestore(stack);
        if (!ret) {
          cursor.delete().onerror = function(e) {
            console.error('delete request failed');
          }
        }
      } else {
        // did offset wrap around?
        if (count != 0) {
          offset = offset % count;
          // recurse
          _emscripten_plugin_get_zero_anonymity_int(offset, type, proc,
              proc_cls, datum_processor_wrapper);
          return;
        }
        // not found
        Runtime.dynCall('iiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_keys_int: function(proc, proc_cls) {
    var index = self.dsdb.transaction(['datastore'], 'readonly')
                    .objectStore('datastore').index('by_key');
    var request = index.openKeyCursor(null, 'nextunique');
    request.onerror = function(e) {
      console.error('cursor request failed');
      Runtime.dynCall('viii', proc, [proc_cls, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var request = index.count(cursor.key);
        request.onerror = function(e) {
          console.error('count request failed');
        };
        request.onsuccess = function(e) {
          ccallFunc(Runtime.getFuncWrapper(proc, 'viii'),
            'void',
            ['number', 'array', 'number'],
            [proc_cls, cursor.key, e.target.result]);
        };
        cursor.continue();
      } else {
        Runtime.dynCall('viii', proc, [proc_cls, 0, 0]);
      }
    };
  }
});

/* vim: set expandtab ts=2 sw=2: */
