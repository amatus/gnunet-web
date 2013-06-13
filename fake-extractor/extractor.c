/*
     This file is part of fake-extractor.
     Copyright (C) 2013 David Barksdale

     fake-extractor is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 3, or (at your
     option) any later version.

     libextractor is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libextractor; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 59 Temple Place - Suite 330,
     Boston, MA 02111-1307, USA.
 */

#include <extractor.h>

/**
 * Load the default set of plugins.  The default can be changed
 * by setting the LIBEXTRACTOR_LIBRARIES environment variable;
 * If it is set to "env", then this function will return
 * EXTRACTOR_plugin_add_config (NULL, env, flags). 
 *
 * If LIBEXTRACTOR_LIBRARIES is not set, the function will attempt
 * to locate the installed plugins and load all of them. 
 * The directory where the code will search for plugins is typically
 * automatically determined; it can be specified explicitly using the
 * "LIBEXTRACTOR_PREFIX" environment variable.  
 *
 * This environment variable must be set to the precise directory with
 * the plugins (i.e. "/usr/lib/libextractor", not "/usr").  Note that
 * setting the environment variable will disable all of the methods
 * that are typically used to determine the location of plugins.
 * Multiple paths can be specified using ':' to separate them.
 *
 * @param flags options for all of the plugins loaded
 * @return the default set of plugins, NULL if no plugins were found
 */
struct EXTRACTOR_PluginList * 
EXTRACTOR_plugin_add_defaults (enum EXTRACTOR_Options flags)
{
	return NULL;
}


/**
 * Add a library for keyword extraction.
 *
 * @param prev the previous list of libraries, may be NULL
 * @param library the name of the library (short handle, i.e. "mime")
 * @param options options to give to the library
 * @param flags options to use
 * @return the new list of libraries, equal to prev iff an error occured
 */
struct EXTRACTOR_PluginList *
EXTRACTOR_plugin_add (struct EXTRACTOR_PluginList * prev,
		      const char *library,
		      const char *options,
		      enum EXTRACTOR_Options flags)
{
	return prev;
}


/**
 * Load multiple libraries as specified by the user.
 *
 * @param config a string given by the user that defines which
 *        libraries should be loaded. Has the format
 *        "[[-]LIBRARYNAME[(options)][:[-]LIBRARYNAME[(options)]]]*".
 *        For example, 'mp3:ogg' loads the
 *        mp3 and the ogg plugins. The '-' before the LIBRARYNAME
 *        indicates that the library should be removed from
 *        the library list.
 * @param prev the  previous list of libraries, may be NULL
 * @param flags options to use
 * @return the new list of libraries, equal to prev iff an error occured
 *         or if config was empty (or NULL).
 */
struct EXTRACTOR_PluginList *
EXTRACTOR_plugin_add_config (struct EXTRACTOR_PluginList *prev,
			     const char *config,
			     enum EXTRACTOR_Options flags)
{
	return prev;
}

		
/**
 * Remove a plugin from a list.
 *
 * @param prev the current list of plugins
 * @param library the name of the plugin to remove (short handle)
 * @return the reduced list, unchanged if the plugin was not loaded
 */
struct EXTRACTOR_PluginList *
EXTRACTOR_plugin_remove (struct EXTRACTOR_PluginList *prev,
			 const char *library)
{
	return prev;
}


/**
 * Remove all plugins from the given list (destroys the list).
 *
 * @param plugin the list of plugins
 */
void 
EXTRACTOR_plugin_remove_all (struct EXTRACTOR_PluginList *plugins)
{
}


/**
 * Extract keywords from a file using the given set of plugins.
 *
 * @param plugins the list of plugins to use
 * @param filename the name of the file, can be NULL if data is not NULL
 * @param data data of the file in memory, can be NULL (in which
 *        case libextractor will open file) if filename is not NULL
 * @param size number of bytes in data, ignored if data is NULL
 * @param proc function to call for each meta data item found
 * @param proc_cls cls argument to proc
 */
void
EXTRACTOR_extract (struct EXTRACTOR_PluginList *plugins,
		   const char *filename,
		   const void *data,
		   size_t size,
		   EXTRACTOR_MetaDataProcessor proc,
		   void *proc_cls)
{
}


/**
 * Simple EXTRACTOR_MetaDataProcessor implementation that simply
 * prints the extracted meta data to the given file.  Only prints
 * those keywords that are in UTF-8 format.
 * 
 * @param handle the file to write to (stdout, stderr), must NOT be NULL,
 *               must be of type "FILE *".
 * @param plugin_name name of the plugin that produced this value
 * @param type libextractor-type describing the meta data
 * @param format basic format information about data 
 * @param data_mime_type mime-type of data (not of the original file);
 *        can be NULL (if mime-type is not known)
 * @param data actual meta-data found
 * @param data_len number of bytes in data
 * @return non-zero if printing failed, otherwise 0.
 */
int 
EXTRACTOR_meta_data_print (void * handle,
			   const char *plugin_name,
			   enum EXTRACTOR_MetaType type,
			   enum EXTRACTOR_MetaFormat format,
			   const char *data_mime_type,
			   const char *data,
			   size_t data_len)
{
	return 0;
}
