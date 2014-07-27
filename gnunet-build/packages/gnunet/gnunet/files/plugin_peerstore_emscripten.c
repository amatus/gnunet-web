/*
 * This file is part of GNUnet
 * (C) 2013-2014  Christian Grothoff (and other contributing authors)
 *
 * GNUnet is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3, or (at your
 * option) any later version.
 *
 * GNUnet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUnet; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file peerstore/plugin_peerstore_emscripten.c
 * @brief IndexedDB-based peerstore backend
 * @author Omar Tarabai
 * @author David Barksdale <amatus@amatus.name>
 */

#include <emscripten.h>
#include "platform.h"
#include "gnunet_peerstore_plugin.h"
#include "gnunet_peerstore_service.h"
#include "peerstore.h"

/**
 * Log an error message at log-level 'level' that indicates
 * a failure of the command 'cmd' on file 'filename'
 * with the message given by strerror(errno).
 */
#define LOG(kind,...) \
  GNUNET_log_from (kind, "peerstore-emscripten", __VA_ARGS__)

/**
 * Context for all functions in this plugin.
 */
struct Plugin
{

  /**
   * Configuration handle
   */
  const struct GNUNET_CONFIGURATION_Handle *cfg;
};

/**
 * Delete expired records (expiry < now)
 *
 * @param cls closure (internal context for the plugin)
 * @param now time to use as reference
 * @return number of records deleted
 */
static int
peerstore_emscripten_expire_records(void *cls,
    struct GNUNET_TIME_Absolute now)
{
  EM_ASM_ARGS({
    var now = $0;
    var store =
      psdb.transaction(['peerstore'], 'readwrite').objectStore('peerstore');
    store.index('by_expiry').openKeyCursor(
        IDBKeyRange.upperBound(now, true)).onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var request = store.delete(cursor.value);
        request.onerror = function(e) {
          Module.print('expiry request failed');
        };
        cursor.continue();
      }
    };
  }, (double)now.abs_value_us);
  return 0;

}

/**
 * Iterate over the records given an optional peer id
 * and/or key.
 *
 * @param cls closure (internal context for the plugin)
 * @param sub_system name of sub system
 * @param peer Peer identity (can be NULL)
 * @param key entry key string (can be NULL)
 * @param iter function to call with the result
 * @param iter_cls closure for @a iter
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on error
 */
static int
peerstore_emscripten_iterate_records (void *cls,
    const char *sub_system,
    const struct GNUNET_PeerIdentity *peer,
    const char *key,
    GNUNET_PEERSTORE_Processor iter, void *iter_cls)
{
  EM_ASM_ARGS({
    var sub_system = Pointer_stringify($0);
    var peer = $1 ? Array.apply([], HEAP8.subarray($1, $1 + 32)) : null;
    var key = $2 ? Pointer_stringify($2) : null;
    var iter = $3;
    var iter_cls = $4;
    var key_range = [sub_system];
    if (peer) {
      key_range.push(peer);
    }
    if (key) {
      key_range.push(key);
    }
    var store =
      psdb.transaction(['peerstore'], 'readonly').objectStore('peerstore');
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
    index.openCursor(IDBKeyRange.only(key_range)).onsuccess = function(e) {
      var cursor = e.target.result;
      if (cursor) {
        var stack = Runtime.stackSave();
        var record = Runtime.stackAlloc(6 * 4);
        var sub_system = allocate(intArrayFromString(cursor.value.sub_system),
          'i8', ALLOC_STACK);
        var peer = allocate(cursor.value.peer, 'i8', ALLOC_STACK);
        var key = allocate(intArrayFromString(cursor.value.key), 'i8',
          ALLOC_STACK);
        var value = allocate(cursor.value.value, 'i8', ALLOC_STACK);
        var expiry = Runtime.stackAlloc(8);
        setValue(expiry, cursor.value.expiry, 'i64');
        setValue(record, sub_system, 'i32');
        setValue(record + 4, peer, 'i32');
        setValue(record + 8, key, 'i32');
        setValue(record + 12, value, 'i32');
        setValue(record + 16, cursor.value.value.length, 'i32');
        setValue(record + 20, expiry, 'i32');
        Runtime.dynCall('viii', iter, [iter_cls, record, 0]);
        Runtime.stackRestore(stack);
        cursor.continue();
      }
    };
  }, sub_system, peer, key, iter, iter_cls);
  return GNUNET_OK;
}

/**
 * Store a record in the peerstore.
 * Key is the combination of sub system and peer identity.
 * One key can store multiple values.
 *
 * @param cls closure (internal context for the plugin)
 * @param peer peer identity
 * @param sub_system name of the GNUnet sub system responsible
 * @param value value to be stored
 * @param size size of value to be stored
 * @return #GNUNET_OK on success, else #GNUNET_SYSERR
 */
static int
peerstore_emscripten_store_record(void *cls,
    const char *sub_system,
    const struct GNUNET_PeerIdentity *peer,
    const char *key,
    const void *value,
    size_t size,
    struct GNUNET_TIME_Absolute expiry,
    enum GNUNET_PEERSTORE_StoreOption options)
{
  EM_ASM_ARGS({
    var sub_system = Pointer_stringify($0);
    var peer = Array.apply([], HEAP8.subarray($1, $1 + 32));
    var key = Pointer_stringify($2);
    var value = new Uint8Array(HEAP8.subarray($3, $3 + $4));
    var expiry = $5;
    var options = $6;
    var store =
      psdb.transaction(['peerstore'], 'readwrite').objectStore('peerstore');
    var put = function() {
      var request = store.put(
          {sub_system: sub_system,
           peer: peer,
           key: key,
           value: value,
           expiry: expiry});
      request.onerror = function(e) {
        Module.print('put request failed');
      };
    };
    if (options == 1) {
      store.index('by_all').openKeyCursor(
          IDBKeyRange.only([sub_system, peer, key])).onsuccess = function(e) {
        var cursor = e.target.result;
        if (cursor) {
          var request = store.delete(cursor.value);
          request.onerror = function(e) {
            Module.print('replace request failed');
          };
          cursor.continue();
        } else {
          put();
        }
      }
    } else {
      put();
    }
  }, sub_system, peer, key, value, size, (double)expiry.abs_value_us, options);
  return GNUNET_OK;
}


/**
 * Initialize the database connections and associated
 * data structures (create tables and indices
 * as needed as well).
 *
 * @param plugin the plugin context (state for this module)
 * @return GNUNET_OK on success
 */
static int
database_setup (struct Plugin *plugin)
{
  EM_ASM({
    var request = indexedDB.open('peerstore', 1);
    request.onsuccess = function(e) {
      psdb = e.target.result;
    };
    request.onerror = function(e) {
      Module.print('Error opening peerstore database');
    };
    request.onupgradeneeded = function(e) {
      var db = e.target.result;
      var store = db.createObjectStore('peerstore', {autoIncrement: true});
      store.createIndex('by_subsystem', 'sub_system');
      store.createIndex('by_pid', ['sub_system', 'peer_id']);
      store.createIndex('by_key', ['sub_system', 'key']);
      store.createIndex('by_all', ['sub_system', 'peer_id', 'key']);
      store.createIndex('by_expiry', 'expiry');
    };
  });
  return GNUNET_OK;
}

/**
 * Shutdown database connection and associate data
 * structures.
 * @param plugin the plugin context (state for this module)
 */
static void
database_shutdown (struct Plugin *plugin)
{
}

/**
 * Entry point for the plugin.
 *
 * @param cls The struct GNUNET_CONFIGURATION_Handle.
 * @return NULL on error, otherwise the plugin context
 */
void *
libgnunet_plugin_peerstore_emscripten_init (void *cls)
{
  static struct Plugin plugin;
  const struct GNUNET_CONFIGURATION_Handle *cfg = cls;
  struct GNUNET_PEERSTORE_PluginFunctions *api;

  if (NULL != plugin.cfg)
    return NULL;                /* can only initialize once! */
  memset (&plugin, 0, sizeof (struct Plugin));
  plugin.cfg = cfg;
  if (GNUNET_OK != database_setup (&plugin))
  {
    database_shutdown (&plugin);
    return NULL;
  }
  api = GNUNET_new (struct GNUNET_PEERSTORE_PluginFunctions);
  api->cls = &plugin;
  api->store_record = &peerstore_emscripten_store_record;
  api->iterate_records = &peerstore_emscripten_iterate_records;
  api->expire_records = &peerstore_emscripten_expire_records;
  LOG(GNUNET_ERROR_TYPE_DEBUG, "emscripten plugin is running\n");
  return api;
}

/**
 * Exit point from the plugin.
 *
 * @param cls The plugin context (as returned by "init")
 * @return Always NULL
 */
void *
libgnunet_plugin_peerstore_emscripten_done (void *cls)
{
  struct GNUNET_PEERSTORE_PluginFunctions *api = cls;
  struct Plugin *plugin = api->cls;

  database_shutdown (plugin);
  plugin->cfg = NULL;
  GNUNET_free (api);
  LOG (GNUNET_ERROR_TYPE_DEBUG, "emscripten plugin is finished\n");
  return NULL;

}

/* vim: set expandtab ts=2 sw=2: */
