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
 * @param absent true if the key was not found in the bloom filter
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
emscripten_plugin_put (void *cls,
                       const struct GNUNET_HashCode *key,
                       bool absent,
                       uint32_t size,
                       const void *data,
                       enum GNUNET_BLOCK_Type type,
                       uint32_t priority,
                       uint32_t anonymity,
                       uint32_t replication,
                       struct GNUNET_TIME_Absolute expiration,
                       PluginPutCont cont,
                       void *cont_cls)
{
  extern void
  emscripten_plugin_put_int(const void *key,
                            double absent,
                            const void *data,
                            double size,
                            double type,
                            double priority,
                            double anonymity,
                            double replication,
                            double expiration,
                            void *cont,
                            void *cons_cls);

  emscripten_plugin_put_int(key, absent, data, size, type, priority, anonymity,
      replication, expiration.abs_value_us, cont, cont_cls);
}


static int
datum_processor_wrapper(PluginDatumProcessor proc,
                        void *proc_cls,
                        const struct GNUNET_HashCode *key,
                        uint32_t size,
                        const void *data,
                        enum GNUNET_BLOCK_Type type,
                        uint32_t priority,
                        uint32_t anonymity,
                        uint32_t replication,
                        double *expiration,
                        uint32_t uid)
{
  struct GNUNET_TIME_Absolute expiry;

  expiry.abs_value_us = *expiration;
  return proc(proc_cls, key, size, data, type, priority, anonymity, replication,
      expiry, uid);
}


/**
 * Get one of the results for a particular key in the datastore.
 *
 * @param cls closure
 * @param next_uid return the result with lowest uid >= next_uid
 * @param random if true, return a random result instead of using next_uid
 * @param key maybe NULL (to match all entries)
 * @param type entries of which type are relevant?
 *     Use 0 for any type.
 * @param proc function to call on each matching value;
 *        will be called with NULL if nothing matches
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_key (void *cls,
                           uint64_t next_uid,
                           bool random,
                           const struct GNUNET_HashCode *key,
                           enum GNUNET_BLOCK_Type type,
                           PluginDatumProcessor proc,
                           void *proc_cls)
{
  extern void emscripten_plugin_get_key_int(double next_uid, double random,
      const void *key, double type, void *proc, void *proc_cls, void *wrapper);

  emscripten_plugin_get_key_int(next_uid, random, key, type, proc, proc_cls,
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
 * Call the given processor on an item with zero anonymity.
 *
 * @param cls our "struct Plugin*"
 * @param next_uid return the result with lowest uid >= next_uid
 * @param type entries of which type should be considered?
 *        Use 0 for any type.
 * @param proc function to call on each matching value;
 *        will be called  with NULL if no value matches
 * @param proc_cls closure for proc
 */
static void
emscripten_plugin_get_zero_anonymity (void *cls,
                                      uint64_t next_uid,
                                      enum GNUNET_BLOCK_Type type,
                                      PluginDatumProcessor proc,
                                      void *proc_cls)
{
  extern void emscripten_plugin_get_zero_anonymity_int(double next_uid,
      double type, void *proc, void *proc_cls, void *wrapper);

  emscripten_plugin_get_zero_anonymity_int(next_uid, type, proc, proc_cls,
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
 * Remove a particular key in the datastore.
 *
 * @param cls closure
 * @param key key for the content
 * @param size number of bytes in data
 * @param data content stored
 * @param cont continuation called with success or failure status
 * @param cont_cls continuation closure for @a cont
 */
static void
emscripten_plugin_remove_key (void *cls,
                              const struct GNUNET_HashCode *key,
                              uint32_t size,
                              const void *data,
                              PluginRemoveCont cont,
                              void *cont_cls)
{
  extern void
  emscripten_plugin_remove_key_int(void *key,
                                   double size,
                                   void *data,
                                   void *cont,
                                   void *cont_cls);

  emscripten_plugin_remove_key_int(key, size, data, cont, cont_cls);
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
  api->get_key = &emscripten_plugin_get_key;
  api->get_replication = &emscripten_plugin_get_replication;
  api->get_expiration = &emscripten_plugin_get_expiration;
  api->get_zero_anonymity = &emscripten_plugin_get_zero_anonymity;
  api->drop = &emscripten_plugin_drop;
  api->get_keys = &emscripten_plugin_get_keys;
  api->remove_key = &emscripten_plugin_remove_key;
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
