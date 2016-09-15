/*
 * This file is part of GNUnet
 * Copyright (C) 2013-2014  Christian Grothoff (and other contributing authors)
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/**
 * @file peerstore/plugin_peerstore_emscripten.c
 * @brief IndexedDB-based peerstore backend
 * @author Omar Tarabai
 * @author David Barksdale <amatus@amatus.name>
 */

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
  extern void peerstore_emscripten_expire_records_int(double now, void *cont,
      void *cont_cls);

  peerstore_emscripten_expire_records_int(now.abs_value_us, cont, cont_cls);
  return GNUNET_OK;
}

/* emscripten has a bug which breaks dynamic calling of a function of a
 * unique type signature, so we pass expiry_dbl as a pointer */
static void
peerstore_emscripten_iter_wrapper (
    GNUNET_PEERSTORE_Processor iter,
    void *iter_cls,
    char *sub_system,
    struct GNUNET_PeerIdentity *peer,
    char *key,
    void *value,
    size_t value_size,
    double *expiry_dbl)
{
  struct GNUNET_TIME_Absolute expiry;
  struct GNUNET_PEERSTORE_Record ret;

  expiry.abs_value_us = *expiry_dbl;
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
  extern void peerstore_emscripten_iterate_records_int(void *sub_system,
      void *peer, void *key, void *iter, void *iter_cls, void *wrapper);

  peerstore_emscripten_iterate_records_int(sub_system, peer, key, iter,
      iter_cls, &peerstore_emscripten_iter_wrapper);
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
  extern void peerstore_emscripten_store_record_int(void *sub_system,
      void *peer, void *key, void *value, double size, double expiry,
      double options, void *cont, void *cont_cls);

  peerstore_emscripten_store_record_int(sub_system, peer, key, value, size,
      expiry.abs_value_us, options, cont, cont_cls);
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
