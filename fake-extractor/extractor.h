/*
     This file is part of libextractor.
     (C) 2002, 2003, 2004, 2005, 2006, 2009, 2012 Vidyut Samanta and Christian Grothoff

     libextractor is free software; you can redistribute it and/or modify
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

#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#ifdef __cplusplus
extern "C" {
#if 0 /* keep Emacsens' auto-indent happy */
}
#endif
#endif


#include <stdint.h>

/**
 * 0.2.6-1 => 0x00020601
 * 4.5.2-0 => 0x04050200
 */
#define EXTRACTOR_VERSION 0x01000000

#include <stdio.h>


/**
 * Options for how plugin execution should be done.
 */
enum EXTRACTOR_Options
  {

    /**
     * Run plugin out-of-process, starting the process once the plugin
     * is to be run.  If a plugin crashes, automatically restart the
     * respective process for the same file and try once more
     * (since the crash may be caused by the previous file).  If
     * the process crashes immediately again, it is not restarted
     * until the next file.
     */
    EXTRACTOR_OPTION_DEFAULT_POLICY = 0,

    /**
     * Deprecated option.  Ignored.
     */
    EXTRACTOR_OPTION_OUT_OF_PROCESS_NO_RESTART = 1,

    /**
     * Run plugins in-process.  Unsafe, not recommended,
     * can be nice for debugging.
     */
    EXTRACTOR_OPTION_IN_PROCESS = 2,

    /**
     * Internal value for plugins that have been disabled.
     */
    EXTRACTOR_OPTION_DISABLED = 3

  };


/**
 * Format in which the extracted meta data is presented.
 */
enum EXTRACTOR_MetaFormat
  {
    /**
     * Format is unknown.
     */
    EXTRACTOR_METAFORMAT_UNKNOWN = 0,

    /**
     * 0-terminated, UTF-8 encoded string.  "data_len"
     * is strlen(data)+1.
     */
    EXTRACTOR_METAFORMAT_UTF8 = 1,

    /**
     * Some kind of binary format, see given Mime type.
     */
    EXTRACTOR_METAFORMAT_BINARY = 2,

    /**
     * 0-terminated string.  The specific encoding is unknown.
     * "data_len" is strlen(data)+1.
     */
    EXTRACTOR_METAFORMAT_C_STRING = 3

  };


/**
 * Enumeration defining various sources of keywords.  See also
 * http://dublincore.org/documents/1998/09/dces/
 */
enum EXTRACTOR_MetaType 
  {
    /* fundamental types */
    EXTRACTOR_METATYPE_RESERVED = 0,
    EXTRACTOR_METATYPE_MIMETYPE = 1,
    EXTRACTOR_METATYPE_FILENAME = 2,
    EXTRACTOR_METATYPE_COMMENT = 3,

    /* Standard types from bibtex */
    EXTRACTOR_METATYPE_TITLE = 4,
    EXTRACTOR_METATYPE_BOOK_TITLE = 5,
    EXTRACTOR_METATYPE_BOOK_EDITION = 6,
    EXTRACTOR_METATYPE_BOOK_CHAPTER_NUMBER = 7,
    EXTRACTOR_METATYPE_JOURNAL_NAME = 8,
    EXTRACTOR_METATYPE_JOURNAL_VOLUME = 9,    
    EXTRACTOR_METATYPE_JOURNAL_NUMBER = 10,
    EXTRACTOR_METATYPE_PAGE_COUNT = 11,
    EXTRACTOR_METATYPE_PAGE_RANGE = 12,
    EXTRACTOR_METATYPE_AUTHOR_NAME = 13,
    EXTRACTOR_METATYPE_AUTHOR_EMAIL = 14,
    EXTRACTOR_METATYPE_AUTHOR_INSTITUTION = 15,
    EXTRACTOR_METATYPE_PUBLISHER = 16,
    EXTRACTOR_METATYPE_PUBLISHER_ADDRESS = 17,
    EXTRACTOR_METATYPE_PUBLISHER_INSTITUTION = 18,
    EXTRACTOR_METATYPE_PUBLISHER_SERIES = 19,
    EXTRACTOR_METATYPE_PUBLICATION_TYPE = 20,
    EXTRACTOR_METATYPE_PUBLICATION_YEAR = 21,
    EXTRACTOR_METATYPE_PUBLICATION_MONTH = 22,
    EXTRACTOR_METATYPE_PUBLICATION_DAY = 23,
    EXTRACTOR_METATYPE_PUBLICATION_DATE = 24,
    EXTRACTOR_METATYPE_BIBTEX_EPRINT = 25,
    EXTRACTOR_METATYPE_BIBTEX_ENTRY_TYPE = 26,
    EXTRACTOR_METATYPE_LANGUAGE = 27,
    EXTRACTOR_METATYPE_CREATION_TIME = 28,
    EXTRACTOR_METATYPE_URL = 29,

    /* "unique" document identifiers */
    EXTRACTOR_METATYPE_URI = 30, 
    EXTRACTOR_METATYPE_ISRC = 31,
    EXTRACTOR_METATYPE_HASH_MD4 = 32,
    EXTRACTOR_METATYPE_HASH_MD5 = 33,
    EXTRACTOR_METATYPE_HASH_SHA0 = 34,
    EXTRACTOR_METATYPE_HASH_SHA1 = 35,
    EXTRACTOR_METATYPE_HASH_RMD160 = 36,

    /* identifiers of a location */
    EXTRACTOR_METATYPE_GPS_LATITUDE_REF = 37,
    EXTRACTOR_METATYPE_GPS_LATITUDE = 38,
    EXTRACTOR_METATYPE_GPS_LONGITUDE_REF = 39,
    EXTRACTOR_METATYPE_GPS_LONGITUDE = 40,
    EXTRACTOR_METATYPE_LOCATION_CITY = 41,
    EXTRACTOR_METATYPE_LOCATION_SUBLOCATION = 42,
    EXTRACTOR_METATYPE_LOCATION_COUNTRY = 43,
    EXTRACTOR_METATYPE_LOCATION_COUNTRY_CODE = 44,

    /* generic attributes */
    EXTRACTOR_METATYPE_UNKNOWN = 45,
    EXTRACTOR_METATYPE_DESCRIPTION = 46,
    EXTRACTOR_METATYPE_COPYRIGHT = 47,
    EXTRACTOR_METATYPE_RIGHTS = 48,
    EXTRACTOR_METATYPE_KEYWORDS = 49,
    EXTRACTOR_METATYPE_ABSTRACT = 50,
    EXTRACTOR_METATYPE_SUMMARY = 51,
    EXTRACTOR_METATYPE_SUBJECT = 52,
    EXTRACTOR_METATYPE_CREATOR = 53,
    EXTRACTOR_METATYPE_FORMAT = 54,
    EXTRACTOR_METATYPE_FORMAT_VERSION = 55,

    /* processing history */
    EXTRACTOR_METATYPE_CREATED_BY_SOFTWARE = 56, 
    EXTRACTOR_METATYPE_UNKNOWN_DATE = 57, 
    EXTRACTOR_METATYPE_CREATION_DATE = 58,
    EXTRACTOR_METATYPE_MODIFICATION_DATE = 59,
    EXTRACTOR_METATYPE_LAST_PRINTED = 60,
    EXTRACTOR_METATYPE_LAST_SAVED_BY = 61,
    EXTRACTOR_METATYPE_TOTAL_EDITING_TIME = 62,
    EXTRACTOR_METATYPE_EDITING_CYCLES = 63,
    EXTRACTOR_METATYPE_MODIFIED_BY_SOFTWARE = 64,
    EXTRACTOR_METATYPE_REVISION_HISTORY = 65,

    EXTRACTOR_METATYPE_EMBEDDED_FILE_SIZE = 66,
    EXTRACTOR_METATYPE_FINDER_FILE_TYPE = 67,
    EXTRACTOR_METATYPE_FINDER_FILE_CREATOR = 68,

    /* software package specifics (deb, rpm, tgz, elf) */
    EXTRACTOR_METATYPE_PACKAGE_NAME = 69,
    EXTRACTOR_METATYPE_PACKAGE_VERSION = 70,
    EXTRACTOR_METATYPE_SECTION = 71,
    EXTRACTOR_METATYPE_UPLOAD_PRIORITY = 72,
    EXTRACTOR_METATYPE_PACKAGE_DEPENDENCY = 73,
    EXTRACTOR_METATYPE_PACKAGE_CONFLICTS = 74,
    EXTRACTOR_METATYPE_PACKAGE_REPLACES = 75,
    EXTRACTOR_METATYPE_PACKAGE_PROVIDES = 76,
    EXTRACTOR_METATYPE_PACKAGE_RECOMMENDS = 77,
    EXTRACTOR_METATYPE_PACKAGE_SUGGESTS = 78,
    EXTRACTOR_METATYPE_PACKAGE_MAINTAINER = 79,
    EXTRACTOR_METATYPE_PACKAGE_INSTALLED_SIZE = 80,
    EXTRACTOR_METATYPE_PACKAGE_SOURCE = 81,
    EXTRACTOR_METATYPE_PACKAGE_ESSENTIAL = 82,
    EXTRACTOR_METATYPE_TARGET_ARCHITECTURE = 83,
    EXTRACTOR_METATYPE_PACKAGE_PRE_DEPENDENCY = 84,
    EXTRACTOR_METATYPE_LICENSE = 85,
    EXTRACTOR_METATYPE_PACKAGE_DISTRIBUTION = 86,
    EXTRACTOR_METATYPE_BUILDHOST = 87,
    EXTRACTOR_METATYPE_VENDOR = 88,
    EXTRACTOR_METATYPE_TARGET_OS = 89,
    EXTRACTOR_METATYPE_SOFTWARE_VERSION = 90,
    EXTRACTOR_METATYPE_TARGET_PLATFORM = 91,
    EXTRACTOR_METATYPE_RESOURCE_TYPE = 92,
    EXTRACTOR_METATYPE_LIBRARY_SEARCH_PATH = 93,
    EXTRACTOR_METATYPE_LIBRARY_DEPENDENCY = 94,
    
    /* photography specifics */
    EXTRACTOR_METATYPE_CAMERA_MAKE = 95,
    EXTRACTOR_METATYPE_CAMERA_MODEL = 96,
    EXTRACTOR_METATYPE_EXPOSURE = 97,
    EXTRACTOR_METATYPE_APERTURE = 98,
    EXTRACTOR_METATYPE_EXPOSURE_BIAS = 99,
    EXTRACTOR_METATYPE_FLASH = 100,
    EXTRACTOR_METATYPE_FLASH_BIAS = 101,
    EXTRACTOR_METATYPE_FOCAL_LENGTH = 102,
    EXTRACTOR_METATYPE_FOCAL_LENGTH_35MM = 103,
    EXTRACTOR_METATYPE_ISO_SPEED = 104,
    EXTRACTOR_METATYPE_EXPOSURE_MODE = 105,
    EXTRACTOR_METATYPE_METERING_MODE = 106,
    EXTRACTOR_METATYPE_MACRO_MODE = 107,
    EXTRACTOR_METATYPE_IMAGE_QUALITY = 108,
    EXTRACTOR_METATYPE_WHITE_BALANCE = 109,
    EXTRACTOR_METATYPE_ORIENTATION = 110,
    EXTRACTOR_METATYPE_MAGNIFICATION = 111,

    /* image specifics */
    EXTRACTOR_METATYPE_IMAGE_DIMENSIONS = 112, 
    EXTRACTOR_METATYPE_PRODUCED_BY_SOFTWARE = 113, 
    EXTRACTOR_METATYPE_THUMBNAIL = 114,
    EXTRACTOR_METATYPE_IMAGE_RESOLUTION = 115,
    EXTRACTOR_METATYPE_SOURCE = 116,

    /* (text) document processing specifics */
    EXTRACTOR_METATYPE_CHARACTER_SET = 117,
    EXTRACTOR_METATYPE_LINE_COUNT = 118,
    EXTRACTOR_METATYPE_PARAGRAPH_COUNT = 119,
    EXTRACTOR_METATYPE_WORD_COUNT = 120,
    EXTRACTOR_METATYPE_CHARACTER_COUNT = 121,
    EXTRACTOR_METATYPE_PAGE_ORIENTATION = 122,
    EXTRACTOR_METATYPE_PAPER_SIZE = 123,
    EXTRACTOR_METATYPE_TEMPLATE = 124,
    EXTRACTOR_METATYPE_COMPANY = 125,
    EXTRACTOR_METATYPE_MANAGER = 126,
    EXTRACTOR_METATYPE_REVISION_NUMBER = 127,

    /* music / video specifics */
    EXTRACTOR_METATYPE_DURATION = 128,
    EXTRACTOR_METATYPE_ALBUM = 129,
    EXTRACTOR_METATYPE_ARTIST = 130,
    EXTRACTOR_METATYPE_GENRE = 131,
    EXTRACTOR_METATYPE_TRACK_NUMBER = 132,
    EXTRACTOR_METATYPE_DISC_NUMBER = 133,
    EXTRACTOR_METATYPE_PERFORMER = 134,
    EXTRACTOR_METATYPE_CONTACT_INFORMATION = 135,
    EXTRACTOR_METATYPE_SONG_VERSION = 136,
    EXTRACTOR_METATYPE_PICTURE = 137,
    EXTRACTOR_METATYPE_COVER_PICTURE = 138,
    EXTRACTOR_METATYPE_CONTRIBUTOR_PICTURE = 139,
    EXTRACTOR_METATYPE_EVENT_PICTURE = 140,
    EXTRACTOR_METATYPE_LOGO = 141,
    EXTRACTOR_METATYPE_BROADCAST_TELEVISION_SYSTEM = 142,
    EXTRACTOR_METATYPE_SOURCE_DEVICE = 143,
    EXTRACTOR_METATYPE_DISCLAIMER = 144,
    EXTRACTOR_METATYPE_WARNING = 145,
    EXTRACTOR_METATYPE_PAGE_ORDER = 146,
    EXTRACTOR_METATYPE_WRITER = 147,
    EXTRACTOR_METATYPE_PRODUCT_VERSION = 148,
    EXTRACTOR_METATYPE_CONTRIBUTOR_NAME = 149,
    EXTRACTOR_METATYPE_MOVIE_DIRECTOR = 150,
    EXTRACTOR_METATYPE_NETWORK_NAME = 151,
    EXTRACTOR_METATYPE_SHOW_NAME = 152,
    EXTRACTOR_METATYPE_CHAPTER_NAME = 153,
    EXTRACTOR_METATYPE_SONG_COUNT = 154,
    EXTRACTOR_METATYPE_STARTING_SONG = 155,
    EXTRACTOR_METATYPE_PLAY_COUNTER = 156,
    EXTRACTOR_METATYPE_CONDUCTOR = 157,
    EXTRACTOR_METATYPE_INTERPRETATION = 158,
    EXTRACTOR_METATYPE_COMPOSER = 159,
    EXTRACTOR_METATYPE_BEATS_PER_MINUTE = 160,
    EXTRACTOR_METATYPE_ENCODED_BY = 161,
    EXTRACTOR_METATYPE_ORIGINAL_TITLE = 162,
    EXTRACTOR_METATYPE_ORIGINAL_ARTIST = 163,
    EXTRACTOR_METATYPE_ORIGINAL_WRITER = 164,
    EXTRACTOR_METATYPE_ORIGINAL_RELEASE_YEAR = 165,
    EXTRACTOR_METATYPE_ORIGINAL_PERFORMER = 166,
    EXTRACTOR_METATYPE_LYRICS = 167,
    EXTRACTOR_METATYPE_POPULARITY_METER = 168,
    EXTRACTOR_METATYPE_LICENSEE = 169,
    EXTRACTOR_METATYPE_MUSICIAN_CREDITS_LIST = 170,
    EXTRACTOR_METATYPE_MOOD = 171, 
    EXTRACTOR_METATYPE_SUBTITLE = 172, 

    /* GNUnet specific values (never extracted) */
    EXTRACTOR_METATYPE_GNUNET_DISPLAY_TYPE = 173,
    EXTRACTOR_METATYPE_GNUNET_FULL_DATA = 174,
    EXTRACTOR_METATYPE_RATING = 175,
    EXTRACTOR_METATYPE_ORGANIZATION = 176,
    EXTRACTOR_METATYPE_RIPPER = 177,
    EXTRACTOR_METATYPE_PRODUCER = 178,
    EXTRACTOR_METATYPE_GROUP = 179,
    EXTRACTOR_METATYPE_GNUNET_ORIGINAL_FILENAME = 180,

    EXTRACTOR_METATYPE_DISC_COUNT = 181,

    EXTRACTOR_METATYPE_CODEC = 182,
    EXTRACTOR_METATYPE_VIDEO_CODEC = 183,
    EXTRACTOR_METATYPE_AUDIO_CODEC = 184,
    EXTRACTOR_METATYPE_SUBTITLE_CODEC = 185,

    EXTRACTOR_METATYPE_CONTAINER_FORMAT = 186,

    EXTRACTOR_METATYPE_BITRATE = 187,
    EXTRACTOR_METATYPE_NOMINAL_BITRATE = 188,
    EXTRACTOR_METATYPE_MINIMUM_BITRATE = 189,
    EXTRACTOR_METATYPE_MAXIMUM_BITRATE = 190,

    EXTRACTOR_METATYPE_SERIAL = 191,

    EXTRACTOR_METATYPE_ENCODER = 192,
    EXTRACTOR_METATYPE_ENCODER_VERSION = 193,

    EXTRACTOR_METATYPE_TRACK_GAIN = 194,
    EXTRACTOR_METATYPE_TRACK_PEAK = 195,
    EXTRACTOR_METATYPE_ALBUM_GAIN = 196,
    EXTRACTOR_METATYPE_ALBUM_PEAK = 197,
    EXTRACTOR_METATYPE_REFERENCE_LEVEL = 198,

    EXTRACTOR_METATYPE_LOCATION_NAME = 199,
    EXTRACTOR_METATYPE_LOCATION_ELEVATION = 200,
    EXTRACTOR_METATYPE_LOCATION_HORIZONTAL_ERROR = 201,
    EXTRACTOR_METATYPE_LOCATION_MOVEMENT_SPEED = 202,
    EXTRACTOR_METATYPE_LOCATION_MOVEMENT_DIRECTION = 203,
    EXTRACTOR_METATYPE_LOCATION_CAPTURE_DIRECTION = 204,

    EXTRACTOR_METATYPE_SHOW_EPISODE_NUMBER = 205,
    EXTRACTOR_METATYPE_SHOW_SEASON_NUMBER = 206,

    EXTRACTOR_METATYPE_GROUPING = 207,

    EXTRACTOR_METATYPE_DEVICE_MANUFACTURER = 208,
    EXTRACTOR_METATYPE_DEVICE_MODEL = 209,

    EXTRACTOR_METATYPE_AUDIO_LANGUAGE = 210,
    EXTRACTOR_METATYPE_CHANNELS = 211,
    EXTRACTOR_METATYPE_SAMPLE_RATE = 212,
    EXTRACTOR_METATYPE_AUDIO_DEPTH = 213,
    EXTRACTOR_METATYPE_AUDIO_BITRATE = 214,
    EXTRACTOR_METATYPE_MAXIMUM_AUDIO_BITRATE = 215,

    EXTRACTOR_METATYPE_VIDEO_DIMENSIONS = 216,
    EXTRACTOR_METATYPE_VIDEO_DEPTH = 217,
    EXTRACTOR_METATYPE_FRAME_RATE = 218,
    EXTRACTOR_METATYPE_PIXEL_ASPECT_RATIO = 219,
    EXTRACTOR_METATYPE_VIDEO_BITRATE = 220,
    EXTRACTOR_METATYPE_MAXIMUM_VIDEO_BITRATE = 221,

    EXTRACTOR_METATYPE_SUBTITLE_LANGUAGE = 222,
    EXTRACTOR_METATYPE_VIDEO_LANGUAGE = 223,

    EXTRACTOR_METATYPE_TOC = 224,

    EXTRACTOR_METATYPE_VIDEO_DURATION = 225,
    EXTRACTOR_METATYPE_AUDIO_DURATION = 226,
    EXTRACTOR_METATYPE_SUBTITLE_DURATION = 227,

    EXTRACTOR_METATYPE_LAST = 228
  };


/**
 * Get the textual name of the keyword.
 *
 * @param type meta type to get a UTF-8 string for
 * @return NULL if the type is not known, otherwise
 *         an English (locale: C) string describing the type;
 *         translate using 'dgettext ("libextractor", rval)'
 */
const char *
EXTRACTOR_metatype_to_string (enum EXTRACTOR_MetaType type);


/**
 * Get a long description for the meta type.
 *
 * @param type meta type to get a UTF-8 description for
 * @return NULL if the type is not known, otherwise
 *         an English (locale: C) string describing the type;
 *         translate using 'dgettext ("libextractor", rval)'
 */
const char *
EXTRACTOR_metatype_to_description (enum EXTRACTOR_MetaType type);


/**
 * Return the highest type number, exclusive as in [0,max).
 *
 * @return highest legal metatype number for this version of libextractor
 */
enum EXTRACTOR_MetaType
EXTRACTOR_metatype_get_max (void);


/**
 * Type of a function that libextractor calls for each
 * meta data item found.
 *
 * @param cls closure (user-defined)
 * @param plugin_name name of the plugin that produced this value;
 *        special values can be used (i.e. '&lt;zlib&gt;' for zlib being
 *        used in the main libextractor library and yielding
 *        meta data).
 * @param type libextractor-type describing the meta data
 * @param format basic format information about data 
 * @param data_mime_type mime-type of data (not of the original file);
 *        can be NULL (if mime-type is not known)
 * @param data actual meta-data found
 * @param data_len number of bytes in data
 * @return 0 to continue extracting, 1 to abort
 */ 
typedef int (*EXTRACTOR_MetaDataProcessor) (void *cls,
					    const char *plugin_name,
					    enum EXTRACTOR_MetaType type,
					    enum EXTRACTOR_MetaFormat format,
					    const char *data_mime_type,
					    const char *data,
					    size_t data_len);


/**
 * Context provided for plugins that perform meta data extraction.
 */
struct EXTRACTOR_ExtractContext
{

  /**
   * Closure argument to pass to all callbacks.
   */
  void *cls;

  /**
   * Configuration string for the plugin.
   */
  const char *config;
  
  /**
   * Obtain a pointer to up to 'size' bytes of data from the file to process.
   *
   * @param cls the 'cls' member of this struct
   * @param data pointer to set to the file data, set to NULL on error
   * @param size maximum number of bytes requested
   * @return number of bytes now available in data (can be smaller than 'size'),
   *         -1 on error
   */
  ssize_t (*read) (void *cls,
		   void **data,
		   size_t size);

  
  /**
   * Seek in the file.  Use 'SEEK_CUR' for whence and 'pos' of 0 to
   * obtain the current position in the file.
   * 
   * @param cls the 'cls' member of this struct
   * @param pos position to seek (see 'man lseek')
   * @param whence how to see (absolute to start, relative, absolute to end)
   * @return new absolute position, -1 on error (i.e. desired position
   *         does not exist)
   */ 
  int64_t (*seek) (void *cls,
		   int64_t pos,
		   int whence);


  /**
   * Determine the overall size of the file.
   * 
   * @param cls the 'cls' member of this struct
   * @return overall file size, UINT64_MAX on error (i.e. IPC failure)
   */ 
  uint64_t (*get_size) (void *cls);

  /**
   * Function to call on extracted data.
   */
  EXTRACTOR_MetaDataProcessor proc;

};


/**
 * Signature of the extract method that each plugin
 * must provide.
 *
 * @param ec extraction context provided to the plugin
 */
typedef void (*EXTRACTOR_extract_method) (struct EXTRACTOR_ExtractContext *ec);


/**
 * Linked list of extractor plugins.  An application builds this list
 * by telling libextractor to load various keyword-extraction
 * plugins. Libraries can also be unloaded (removed from this list,
 * see EXTRACTOR_plugin_remove).
 */
struct EXTRACTOR_PluginList;


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
EXTRACTOR_plugin_add_defaults (enum EXTRACTOR_Options flags);


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
		      enum EXTRACTOR_Options flags);


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
			     enum EXTRACTOR_Options flags);

		
/**
 * Remove a plugin from a list.
 *
 * @param prev the current list of plugins
 * @param library the name of the plugin to remove (short handle)
 * @return the reduced list, unchanged if the plugin was not loaded
 */
struct EXTRACTOR_PluginList *
EXTRACTOR_plugin_remove (struct EXTRACTOR_PluginList *prev,
			 const char *library);


/**
 * Remove all plugins from the given list (destroys the list).
 *
 * @param plugin the list of plugins
 */
void 
EXTRACTOR_plugin_remove_all (struct EXTRACTOR_PluginList *plugins);


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
		   void *proc_cls);


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
			   size_t data_len);


#if 0 /* keep Emacsens' auto-indent happy */
{
#endif
#ifdef __cplusplus
}
#endif

#endif
