/*
 * This file is part of GNUnet
 * (C) 2013 Christian Grothoff (and other contributing authors)
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
 * @file peerstore/plugin_peerstore_sqlite.c
 * @brief sqlite-based peerstore backend
 * @author Omar Tarabai
 */

#include "platform.h"
#include "gnunet_peerstore_plugin.h"
#include "gnunet_peerstore_service.h"
#include "peerstore.h"

/**
 * After how many ms "busy" should a DB operation fail for good?  A
 * low value makes sure that we are more responsive to requests
 * (especially PUTs).  A high value guarantees a higher success rate
 * (SELECTs in iterate can take several seconds despite LIMIT=1).
 *
 * The default value of 1s should ensure that users do not experience
 * huge latencies while at the same time allowing operations to
 * succeed with reasonable probability.
 */
#define BUSY_TIMEOUT_MS 1000

/**
 * Log an error message at log-level 'level' that indicates
 * a failure of the command 'cmd' on file 'filename'
 * with the message given by strerror(errno).
 */
#define LOG_SQLITE(db, level, cmd) do { GNUNET_log_from (level, "peerstore-sqlite", _("`%s' failed at %s:%d with error: %s\n"), cmd, __FILE__, __LINE__, sqlite3_errmsg(db->dbh)); } while(0)

#define LOG(kind,...) GNUNET_log_from (kind, "peerstore-sqlite", __VA_ARGS__)

/**
 * Context for all functions in this plugin.
 */
struct Plugin
{

  /**
   * Configuration handle
   */
  const struct GNUNET_CONFIGURATION_Handle *cfg;

  /**
   * Database filename.
   */
  char *fn;

  /**
   * Native SQLite database handle.
   */
  //XXX sqlite3 *dbh;

  /**
   * Precompiled SQL for inserting into peerstoredata
   */
  //XXX sqlite3_stmt *insert_peerstoredata;

  /**
   * Precompiled SQL for selecting from peerstoredata
   */
  //XXX sqlite3_stmt *select_peerstoredata;

  /**
   * Precompiled SQL for selecting from peerstoredata
   */
  //XXX sqlite3_stmt *select_peerstoredata_by_pid;

  /**
   * Precompiled SQL for selecting from peerstoredata
   */
  //XXX sqlite3_stmt *select_peerstoredata_by_key;

  /**
   * Precompiled SQL for selecting from peerstoredata
   */
  //XXX sqlite3_stmt *select_peerstoredata_by_all;

  /**
   * Precompiled SQL for deleting expired
   * records from peerstoredata
   */
  //XXX sqlite3_stmt *expire_peerstoredata;

  /**
   * Precompiled SQL for deleting records
   * with given key
   */
  //XXX sqlite3_stmt *delete_peerstoredata;

};

/**
 * Delete records with the given key
 *
 * @param cls closure (internal context for the plugin)
 * @param sub_system name of sub system
 * @param peer Peer identity (can be NULL)
 * @param key entry key string (can be NULL)
 * @return number of deleted records
 */
static int
peerstore_emscripten_delete_records(void *cls,
    const char *sub_system,
    const struct GNUNET_PeerIdentity *peer,
    const char *key)
{
  struct Plugin *plugin = cls;
  return 0;
}

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
  struct Plugin *plugin = cls;
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
  struct Plugin *plugin = cls;
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
  struct Plugin *plugin = cls;
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

/* end of plugin_peerstore_emscripten.c */
