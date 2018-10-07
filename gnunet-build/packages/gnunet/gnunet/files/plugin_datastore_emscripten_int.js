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
  emscripten_plugin_put_int: function(key_pointer, absent, data_pointer, size,
                                      type, priority, anonymity, replication,
                                      expiry, cont, cont_cls) {
    var key = Array.prototype.slice.call(HEAP8.subarray(key_pointer,
          key_pointer + 64));
    var data = Array.prototype.slice.call(HEAP8.subarray(data_pointer,
          data_pointer + size));
    var transaction = self.dsdb.transaction(['datastore'], 'readwrite');
    var do_put = function() {
      // workaround for https://crbug.com/701972
      var request = transaction.objectStore('datastore').add({})
      request.onerror = function(e) {
        console.error('add request failed');
        dynCall('viiiii', cont, [cont_cls, key_pointer, size, -1, 0]);
      };
      request.onsuccess = function(e) {
        var request2 = transaction.objectStore('datastore')
                                  .put({uid: e.target.result,
                                        key: key,
                                        data: data,
                                        type: type,
                                        priority: priority,
                                        anonymity: anonymity,
                                        replication: replication,
                                        expiry: expiry});
        request2.onerror = function(e) {
          console.error('put request failed');
          transaction.abort();
          dynCall('viiiii', cont, [cont_cls, key_pointer, size, -1, 0]);
        };
        request2.onsuccess = function(e) {
          dynCall('viiiii', cont, [cont_cls, key_pointer, size, 1, 0]);
        };
      };
    };
    if (absent) {
      do_put();
    } else {
      var request = transaction.objectStore('datastore').index('by_key')
                               .openCursor(IDBKeyRange.bound(
                                 [key], [key, Number.MAX_VALUE]));
      request.onsuccess = function(e) {
        var cursor = e.target.result;
        if (cursor) {
          var value = cursor.value;
          // filter by data
          if (size != value.data.length
              || !data.every(function(x, i) {
                                return x == value.data[i]
                              })) {
            cursor.continue();
            return;
          }
          value.priority += priority;
          value.replication += replication;
          if (value.expiry < expiry) {
            value.expiry = expiry;
          }
          var request = transaction.objectStore('datastore').put(value);
          request.onerror = function(e) {
            console.error('put request failed');
            dynCall('viiiii', cont,
              [cont_cls, key_pointer, size, -1, 0]);
          };
          request.onsuccess = function(e) {
            dynCall('viiiii', cont,
              [cont_cls, key_pointer, size, 0, 0]);
          };
        } else {
          do_put();
        }
      };
    }
  }, emscripten_plugin_get_key_int: function(next_uid, random, key_pointer,
                                             type, proc, proc_cls,
                                             datum_processor_wrapper) {
    var key = key_pointer ? Array.prototype.slice.call(HEAP8.subarray(
          key_pointer, key_pointer + 64)) : null;
    var range = null;
    var transaction = self.dsdb.transaction(['datastore'], 'readonly');
    // TODO: random
    var request;
    if (key_pointer) {
      request = transaction.objectStore('datastore').index('by_key')
                           .openCursor(IDBKeyRange.bound(
                             [key, next_uid],
                             [key, Number.MAX_VALUE]));
    } else {
      request = transaction.objectStore('datastore').openCursor();
    }
    request.onerror = function(e) {
      console.error('cursor request failed');
      dynCall('iiiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var value = cursor.value;
        // optional filter by type
        if (type && type != value.type) {
          cursor.continue();
          return;
        }
        // got a result
        var stack = stackSave();
        var expiry = stackAlloc(getNativeTypeSize('double'));
        setValue(expiry, value.expiry, 'double');
        var ret = ccallFunc(
            getFuncWrapper(datum_processor_wrapper,
              'iiiiiiiiiiii'),
            'number',
            ['number', 'number', 'array', 'number', 'array', 'number', 'number',
             'number', 'number', 'number', 'number'],
            [proc, proc_cls, value.key, value.data.length, value.data,
             value.type, value.priority, value.anonymity, value.replication,
             expiry, value.uid]);
        stackRestore(stack);
      } else {
        // do we need to wrap around?
        if (next_uid != 0) {
          // recurse
          _emscripten_plugin_get_key_int(0, false, key_pointer, type, proc,
            proc_cls, datum_processor_wrapper);
          return;
        }
        // not found
        dynCall('iiiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_replication_int: function(proc, proc_cls,
      datum_processor_wrapper) {
    var transaction = self.dsdb.transaction(['datastore'], 'readwrite');
    var request = transaction.objectStore('datastore').index('by_replication')
                             .openCursor(null, 'prev');
    request.onerror = function(e) {
      console.error('cursor request failed');
      dynCall('iiiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var value = cursor.value
        if (value.replication > 0) {
          --value.replication;
        }
        var request = cursor.update(value);
        request.onerror = function(e) {
          console.error('replication update request failed');
          dynCall('iiiiiiiiiiii', proc,
            [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
        };
        request.onsuccess = function(e) {
          var stack = stackSave();
          var expiry = stackAlloc(getNativeTypeSize('double'));
          setValue(expiry, cursor.value.expiry, 'double');
          var ret = ccallFunc(
              getFuncWrapper(datum_processor_wrapper, 'iiiiiiiiiiii'),
              'number',
              ['number', 'number', 'array', 'number', 'array', 'number',
               'number', 'number', 'number', 'number', 'number'],
              [proc, proc_cls, value.key, value.data.length, value.data,
               value.type, value.priority, value.anonymity, value.replication,
               expiry, value.uid]);
          stackRestore(stack);
        };
      } else {
        dynCall('iiiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_expiration_int: function(proc, proc_cls, now,
      datum_processor_wrapper) {
    var request = self.dsdb.transaction(['datastore'], 'readwrite')
                      .objectStore('datastore').index('by_expiry')
                      .openCursor(IDBKeyRange.upperBound(now, true));
    request.onerror = function(e) {
      console.error('cursor request failed');
      dynCall('iiiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        // got a result
        var stack = stackSave();
        var expiry = stackAlloc(getNativeTypeSize('double'));
        setValue(expiry, cursor.value.expiry, 'double');
        var ret = ccallFunc(
            getFuncWrapper(datum_processor_wrapper,
              'iiiiiiiiiiii'),
            'number',
            ['number', 'number', 'array', 'number', 'array', 'number', 'number',
             'number', 'number', 'number', 'number'],
            [proc, proc_cls, cursor.value.key, cursor.value.data.length,
             cursor.value.data, cursor.value.type, cursor.value.priority,
             cursor.value.anonymity, cursor.value.replication, expiry,
             cursor.value.uid]);
        stackRestore(stack);
        if (!ret) {
          cursor.delete().onerror = function(e) {
            console.error('delete request failed');
          }
        }
      } else {
        dynCall('iiiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_zero_anonymity_int: function(next_uid, type, proc,
      proc_cls, datum_processor_wrapper) {
    var request = self.dsdb.transaction(['datastore'], 'readonly')
                      .objectStore('datastore').index('by_anon_type')
                      .openCursor(IDBKeyRange.bound(
                        [0, type, next_uid], [0, type, Number.MAX_VALUE]));
    request.onerror = function(e) {
      console.error('cursor request failed');
      dynCall('iiiiiiiiiiii', proc,
        [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var stack = stackSave();
        var expiry = stackAlloc(getNativeTypeSize('double'));
        setValue(expiry, cursor.value.expiry, 'double');
        var ret = ccallFunc(
            getFuncWrapper(datum_processor_wrapper,
              'iiiiiiiiiiii'),
            'number',
            ['number', 'number', 'array', 'number', 'array', 'number', 'number',
             'number', 'number', 'number', 'number'],
            [proc, proc_cls, cursor.value.key, cursor.value.data.length,
             cursor.value.data, cursor.value.type, cursor.value.priority,
             cursor.value.anonymity, cursor.value.replication, expiry,
             cursor.value.uid]);
        stackRestore(stack);
      } else {
        // do we need to wrap around?
        if (next_uid != 0) {
          // recurse
          _emscripten_plugin_get_zero_anonymity_int(0, type, proc, proc_cls,
            datum_processor_wrapper);
          return;
        }
        // not found
        dynCall('iiiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, emscripten_plugin_get_keys_int: function(proc, proc_cls) {
    var index = self.dsdb.transaction(['datastore'], 'readonly')
                    .objectStore('datastore').index('by_key');
    var request = index.openKeyCursor(null, 'nextunique');
    request.onerror = function(e) {
      console.error('cursor request failed');
      dynCall('viii', proc, [proc_cls, 0, 0]);
    };
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var request = index.count(cursor.key);
        request.onerror = function(e) {
          console.error('count request failed');
        };
        request.onsuccess = function(e) {
          ccallFunc(getFuncWrapper(proc, 'viii'),
            'void',
            ['number', 'array', 'number'],
            [proc_cls, cursor.key, e.target.result]);
        };
        cursor.continue();
      } else {
        dynCall('viii', proc, [proc_cls, 0, 0]);
      }
    };
  }, emscripten_plugin_remove_key_int: function(key_pointer, size, data_pointer,
                                                cont, cont_cls) {
    var key = Array.prototype.slice.call(HEAP8.subarray(key_pointer,
      key_pointer + 64));
    var data = Array.prototype.slice.call(HEAP8.subarray(data_pointer,
      data_pointer + size));
    var transaction = self.dsdb.transaction(['datastore'], 'readwrite');
    var request = transaction.objectStore('datastore').index('by_key')
                             .openCursor(IDBKeyRange.bound(
                               [key], [key, Number.MAX_VALUE]));
    request.onerror = function(e) {
      console.error('cursor request failed');
      dynCall('viiiii', cont, [cont_cls, key_pointer, size, -1, 0]);
    }
    request.onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var value = cursor.value;
        // filter by data
        if (size != value.data.length
            || !data.every(function(x, i) {
                              return x == value.data[i]
                            })) {
          cursor.continue();
          return;
        }
        // found
        request = cursor.delete();
        request.onerror = function(e) {
          console.error('delete request failed');
          dynCall('viiiii', cont, [cont_cls, key_pointer, size, -1, 0]);
        }
        request.onsuccess = function(e) {
          // removed
          dynCall('viiiii', cont, [cont_cls, key_pointer, size, 1, 0]);
        }
      } else {
        // not found
        dynCall('viiiii', cont, [cont_cls, key_pointer, size, 0, 0]);
      }
    }
  }
});

/* vim: set expandtab ts=2 sw=2: */
