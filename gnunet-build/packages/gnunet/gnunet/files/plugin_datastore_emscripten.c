/*
     This file is part of GNUnet
     Copyright (C) 2009, 2011 Christian Grothoff (and other contributing authors)

     GNUnet is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 3, or (at your
     option) any later version.

     GNUnet is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with GNUnet; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 59 Temple Place - Suite 330,
     Boston, MA 02111-1307, USA.
*/

/**
 * @file datastore/plugin_datastore_emscripten.c
 * @brief IndexedDB-based datastore backend
 * @author Christian Grothoff
 * @author David Barksdale <amatus@amatus.name>
 */

#include <emscripten.h>
#include "platform.h"
#include "gnunet_datastore_plugin.h"


/**
 * Context for all functions in this plugin.
 */
struct Plugin
{
  /**
   * Our execution environment.
   */
  struct GNUNET_DATASTORE_PluginEnvironment *env;
};


/**
 * Get an estimate of how much space the database is
 * currently using.
 *
 * @param cls our "struct Plugin*"
 * @param estimate location to store estimate
 */
static void
emscripten_plugin_estimate_size (void *cls, unsigned long long *estimate)
{
  if (NULL == estimate)
    return;
  /* We're so effecient we can store anything in 0-bytes */
  *estimate = 0;
}


/**
 * Store an item in the datastore.
 *
 * @param cls closure
 * @param key key for the item
 * @param size number of bytes in data
 * @param data content stored
 * @param type type of the content
 * @param priority priority of the content
 * @param anonymity anonymity-level for the content
 * @param replication replication-level for the content
 * @param expiration expiration time for the content
 * @param cont continuation called with success or failure status
 * @param cont_cls continuation closure
 */
static void
emscripten_plugin_put (void *cls, const struct GNUNET_HashCode *key,
                       uint32_t size, const void *data,
                       enum GNUNET_BLOCK_Type type, uint32_t priority,
                       uint32_t anonymity, uint32_t replication,
                       struct GNUNET_TIME_Absolute expiration,
                       PluginPutCont cont, void *cont_cls)
{
  struct GNUNET_HashCode vhash;

  GNUNET_CRYPTO_hash (data, size, &vhash);
  EM_ASM_ARGS({
    var key_pointer = $0;
    var key = Array.prototype.slice.call(HEAP8.subarray($0, $0 + 64));
    var data = Array.prototype.slice.call(HEAP8.subarray($1, $1 + $2));
    var size = $2;
    var type = $3;
    var priority = $4;
    var anonymity = $5;
    var replication = $6;
    var expiry = $7;
    var vhash = Array.prototype.slice.call(HEAP8.subarray($8, $8 + 64));
    var cont = $9;
    var cont_cls = $10;
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
  }, key, data, size, type, priority, anonymity, replication,
     (double)expiration.abs_value_us, &vhash, cont, cont_cls);
}


static int
datum_processor_wrapper(PluginDatumProcessor proc, void *proc_cls,
                        const struct GNUNET_HashCode *key, uint32_t size,
                        const void *data, enum GNUNET_BLOCK_Type type,
                        uint32_t priority, uint32_t anonymity,
                        double *expiration, uint32_t uid)
{
  struct GNUNET_TIME_Absolute expiry;

  expiry.abs_value_us = *expiration;
  return proc(proc_cls, key, size, data, type, priority, anonymity, expiry,
              uid);
}


/**
 * Get one of the results for a particular key in the datastore.
 *
 * @param cls closure
 * @param offset offset of the result (modulo num-results);
 *               specific ordering does not matter for the offset
 * @param key maybe NULL (to match all entries)
 * @param vhash hash of the value, maybe NULL (to
 *        match all values that have the right key).
 *        Note that for DBlocks there is no difference
 *        betwen key and vhash, but for other blocks
 *        there may be!
 * @param type entries of which type are relevant?
 *     Use 0 for any type.
 * @param proc function to call on each matching value;
 *        will be called with NULL if nothing matches
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_key (void *cls, uint64_t offset,
                           const struct GNUNET_HashCode *key,
                           const struct GNUNET_HashCode *vhash,
                           enum GNUNET_BLOCK_Type type,
                           PluginDatumProcessor proc, void *proc_cls)
{
  EM_ASM_ARGS({
    var offset = $0;
    var key_pointer = $1;
    var key = $1 ? Array.prototype.slice.call(HEAP8.subarray($1, $1 + 64))
                 : null;
    var vhash_pointer = $2;
    var vhash = $2 ? Array.prototype.slice.call(HEAP8.subarray($2, $2 + 64))
                   : null;
    var type = $3;
    var proc = $4;
    var proc_cls = $5;
    var datum_processor_wrapper = $6;
    var emscripten_plugin_get_key = $7;
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
          ccallFunc(Runtime.getFuncWrapper(emscripten_plugin_get_key,
                'viiiiiiii'),
              'void',
              ['number', 'number', 'number', 'number', 'number', 'number',
               'number', 'number'],
              [0, offset, 0, key_pointer, vhash_pointer, type, proc, proc_cls]);
          return;
        }
        // not found
        Runtime.dynCall('iiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, (double)offset, key, vhash, type, proc, proc_cls,
     &datum_processor_wrapper, &emscripten_plugin_get_key);
}


/**
 * Get a random item for replication.  Returns a single, not expired,
 * random item from those with the highest replication counters.  The
 * item's replication counter is decremented by one IF it was positive
 * before.  Call 'proc' with all values ZERO or NULL if the datastore
 * is empty.
 *
 * @param cls closure
 * @param proc function to call the value (once only).
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_replication (void *cls, PluginDatumProcessor proc,
                                   void *proc_cls)
{
  EM_ASM_ARGS({
    var proc = $0;
    var proc_cls = $1;
    var datum_processor_wrapper = $2;
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
  }, proc, proc_cls, &datum_processor_wrapper);
}


/**
 * Get a random item for expiration.  Call 'proc' with all values ZERO
 * or NULL if the datastore is empty.
 *
 * @param cls closure
 * @param proc function to call the value (once only).
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_expiration (void *cls, PluginDatumProcessor proc,
                                void *proc_cls)
{
  struct GNUNET_TIME_Absolute now = GNUNET_TIME_absolute_get();
  EM_ASM_ARGS({
    var proc = $0;
    var proc_cls = $1;
    var now = $2;
    var datum_processor_wrapper = $3;
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
  }, proc, proc_cls, (double)now.abs_value_us, &datum_processor_wrapper);
}


/**
 * Update the priority for a particular key in the datastore.  If
 * the expiration time in value is different than the time found in
 * the datastore, the higher value should be kept.  The specified
 * priority should be added to the existing priority, ignoring the
 * priority in value.
 *
 * Note that it is possible for multiple values to match this put.
 * In that case, all of the respective values are updated.
 *
 * @param cls our "struct Plugin*"
 * @param uid unique identifier of the datum
 * @param delta by how much should the priority
 *     change?  If priority + delta < 0 the
 *     priority should be set to 0 (never go
 *     negative).
 * @param expire new expiration time should be the
 *     MAX of any existing expiration time and
 *     this value
 * @param cont continuation called with success or failure status
 * @param cons_cls continuation closure
 */
static void
emscripten_plugin_update (void *cls, uint64_t uid, int delta,
                        struct GNUNET_TIME_Absolute expire,
                        PluginUpdateCont cont, void *cont_cls)
{
  EM_ASM_ARGS({
    var uid = $0;
    var delta = $1;
    var expiry = $2;
    var cont = $3;
    var cont_cls = $4;
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
  }, (uint32_t)uid, delta, (double)expire.abs_value_us, cont, cont_cls);
}


/**
 * Call the given processor on an item with zero anonymity.
 *
 * @param cls our "struct Plugin*"
 * @param offset offset of the result (modulo num-results);
 *               specific ordering does not matter for the offset
 * @param type entries of which type should be considered?
 *        Use 0 for any type.
 * @param proc function to call on each matching value;
 *        will be called  with NULL if no value matches
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_zero_anonymity (void *cls, uint64_t offset,
                                    enum GNUNET_BLOCK_Type type,
                                    PluginDatumProcessor proc, void *proc_cls)
{
  EM_ASM_ARGS({
    var offset = $0;
    var type = $1;
    var proc = $2;
    var proc_cls = $3;
    var datum_processor_wrapper = $4;
    var emscripten_plugin_get_zero_anonymity = $5;
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
          ccallFunc(Runtime.getFuncWrapper(emscripten_plugin_get_zero_anonymity,
                'viiiiii'),
              'void',
              ['number', 'number', 'number', 'number', 'number', 'number'],
              [0, offset, 0, type, proc, proc_cls]);
          return;
        }
        // not found
        Runtime.dynCall('iiiiiiiiiii', proc,
          [proc_cls, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
      }
    };
  }, (double)offset, type, proc, proc_cls, &datum_processor_wrapper,
     &emscripten_plugin_get_zero_anonymity);
}


/**
 * Drop database.
 */
static void
emscripten_plugin_drop (void *cls)
{
  // This is only used in tests
}


/**
 * Get all of the keys in the datastore.
 *
 * @param cls closure
 * @param proc function to call on each key
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_keys (void *cls,
		   PluginKeyProcessor proc,
		   void *proc_cls)
{
  EM_ASM_ARGS({
    var proc = $0;
    var proc_cls = $1;
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
  }, proc, proc_cls);
}


/**
 * Entry point for the plugin.
 *
 * @param cls the "struct GNUNET_DATASTORE_PluginEnvironment*"
 * @return our "struct Plugin*"
 */
void *
libgnunet_plugin_datastore_emscripten_init (void *cls)
{
  struct GNUNET_DATASTORE_PluginEnvironment *env = cls;
  struct GNUNET_DATASTORE_PluginFunctions *api;
  struct Plugin *plugin;

  plugin = GNUNET_new (struct Plugin);
  plugin->env = env;
  api = GNUNET_new (struct GNUNET_DATASTORE_PluginFunctions);
  api->cls = plugin;
  api->estimate_size = &emscripten_plugin_estimate_size;
  api->put = &emscripten_plugin_put;
  api->update = &emscripten_plugin_update;
  api->get_key = &emscripten_plugin_get_key;
  api->get_replication = &emscripten_plugin_get_replication;
  api->get_expiration = &emscripten_plugin_get_expiration;
  api->get_zero_anonymity = &emscripten_plugin_get_zero_anonymity;
  api->drop = &emscripten_plugin_drop;
  api->get_keys = &emscripten_plugin_get_keys;
  GNUNET_log_from (GNUNET_ERROR_TYPE_INFO, "emscripten",
                   _("Emscripten database running\n"));
  return api;
}


/**
 * Exit point from the plugin.
 * @param cls our "struct Plugin*"
 * @return always NULL
 */
void *
libgnunet_plugin_datastore_emscripten_done (void *cls)
{
  struct GNUNET_DATASTORE_PluginFunctions *api = cls;
  struct Plugin *plugin = api->cls;

  GNUNET_free (plugin);
  GNUNET_free (api);
  return NULL;
}

/* vim: set expandtab ts=2 sw=2: */
