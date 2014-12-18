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
 * @param cont continuation called with the number of records expired
 * @param cont_cls continuation closure
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on error and cont is not
 * called
 */
static int
peerstore_emscripten_expire_records(void *cls,
    struct GNUNET_TIME_Absolute now,
    GNUNET_PEERSTORE_Continuation cont,
    void *cont_cls)
{
  EM_ASM_ARGS({
    var now = $0;
    var cont = $1;
    var cont_cls = $2;
    var count = 0;
    var store =
     self.psdb.transaction(['peerstore'], 'readwrite').objectStore('peerstore');
    var request = store.index('by_expiry').openKeyCursor(
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
  }, (double)now.abs_value_us, cont, cont_cls);
  return GNUNET_OK;
}

static void
peerstore_emscripten_iter_wrapper (
    GNUNET_PEERSTORE_Processor iter,
    void *iter_cls,
    char *sub_system,
    struct GNUNET_PeerIdentity *peer,
    char *key,
    void *value,
    size_t value_size,
    double expiry_dbl)
{
  struct GNUNET_TIME_Absolute expiry;
  struct GNUNET_PEERSTORE_Record ret;

  expiry.abs_value_us = expiry_dbl;
  ret.sub_system = sub_system;
  ret.peer = peer;
  ret.key = key;
  ret.value = value;
  ret.value_size = value_size;
  ret.expiry = &expiry;
  iter (iter_cls, &ret, NULL);
}

/**
 * Iterate over the records given an optional peer id
 * and/or key.
 *
 * @param cls closure (internal context for the plugin)
 * @param sub_system name of sub system
 * @param peer Peer identity (can be NULL)
 * @param key entry key string (can be NULL)
 * @param iter function to call asynchronously with the results, terminated
 * by a NULL result
 * @param iter_cls closure for @a iter
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on error and iter is not
 * called
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
    var peer = $1 ? Array.prototype.slice.call(HEAP8.subarray($1, $1 + 32))
                  : null;
    var key = $2 ? Pointer_stringify($2) : null;
    var iter = $3;
    var iter_cls = $4;
    var peerstore_emscripten_iter_wrapper = $5;
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
        ccallFunc(peerstore_emscripten_iter_wrapper, "number"
            ["number", "number", "string", "array", "string", "array", "number",
             "number"],
            [iter, iter_cls, cursor.value.subsystem, cursor.value.peer,
             cursor.value.key, cursor.value.value, cursor.value.value.length,
             cursor.value.expiry]);
        cursor.continue();
      } else {
        Runtime.dynCall('iiii', iter, [iter_cls, 0, 0]);
      }
    };
    request.onerror = function(e) {
      Module.print('cursor request failed');
      Runtime.dynCall('iiii', iter, [iter_cls, 0, -1]);
    };
  }, sub_system, peer, key, iter, iter_cls, &peerstore_emscripten_iter_wrapper);
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
 * @param cont continuation called when record is stored
 * @param cont_cls continuation closure
 * @return #GNUNET_OK on success, else #GNUNET_SYSERR and cont is not called
 */
static int
peerstore_emscripten_store_record(void *cls,
    const char *sub_system,
    const struct GNUNET_PeerIdentity *peer,
    const char *key,
    const void *value,
    size_t size,
    struct GNUNET_TIME_Absolute expiry,
    enum GNUNET_PEERSTORE_StoreOption options,
    GNUNET_PEERSTORE_Continuation cont,
    void *cont_cls)
{
  EM_ASM_ARGS({
    var sub_system = Pointer_stringify($0);
    var peer = Array.prototype.slice.call(HEAP8.subarray($1, $1 + 32));
    var key = Pointer_stringify($2);
    var value = new Uint8Array(HEAP8.subarray($3, $3 + $4));
    var expiry = $5;
    var options = $6;
    var cont = $7;
    var cont_cls = $8;
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
        Runtime.dynCall('vii', cont, [cont_cls, 0]);
      };
    };
    if (options == 1) {
      var request = store.index('by_all').openKeyCursor(
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
  }, sub_system, peer, key, value, size, (double)expiry.abs_value_us, options,
     cont, cont_cls);
  return GNUNET_OK;
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

  plugin->cfg = NULL;
  GNUNET_free (api);
  LOG (GNUNET_ERROR_TYPE_DEBUG, "emscripten plugin is finished\n");
  return NULL;

}

/* vim: set expandtab ts=2 sw=2: */
