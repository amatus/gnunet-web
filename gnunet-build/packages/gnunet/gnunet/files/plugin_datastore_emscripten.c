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
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
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
  extern void emscripten_plugin_put_int(const void *key, const void *data,
      int size, int type, int priority, int anonymity, int replication,
      double expiration, void *vhash, void *cont, void *cons_cls);
  struct GNUNET_HashCode vhash;

  GNUNET_CRYPTO_hash (data, size, &vhash);
  emscripten_plugin_put_int(key, data, size, type, priority, anonymity,
      replication, expiration.abs_value_us, &vhash, cont, cont_cls);
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
  extern void emscripten_plugin_get_key_int(double offset, const void *key,
      const void *vhash, int type, void *proc, void *proc_cls, void *wrapper);

  emscripten_plugin_get_key_int(offset, key, vhash, type, proc, proc_cls,
      &datum_processor_wrapper);
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
  extern void emscripten_plugin_get_replication_int(void *proc, void *proc_cls,
      void *wrapper);

  emscripten_plugin_get_replication_int(proc, proc_cls,
      &datum_processor_wrapper);
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
  extern void emscripten_plugin_get_expiration_int(void *proc, void *proc_cls,
      double now, void *wrapper);
  struct GNUNET_TIME_Absolute now = GNUNET_TIME_absolute_get();

  emscripten_plugin_get_expiration_int(proc, proc_cls, now.abs_value_us,
      &datum_processor_wrapper);
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
  extern void emscripten_plugin_update_int(int uid, int delta, double expriy,
      void *cont, void *cont_cls);

  emscripten_plugin_update_int(uid, delta, expire.abs_value_us, cont, cont_cls);
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
  extern void emscripten_plugin_get_zero_anonymity_int(int offset, int type,
      void *proc, void *proc_cls, void *wrapper);

  emscripten_plugin_get_zero_anonymity_int(offset, type, proc, proc_cls,
      &datum_processor_wrapper);
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
  extern void emscripten_plugin_get_keys_int(void *proc, void *proc_cls);

  emscripten_plugin_get_keys_int(proc, proc_cls);
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

  api = GNUNET_new (struct GNUNET_DATASTORE_PluginFunctions);
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

  GNUNET_free (api);
  return NULL;
}

/* vim: set expandtab ts=2 sw=2: */
