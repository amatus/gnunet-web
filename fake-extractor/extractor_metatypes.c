/*
     This file is part of libextractor.
     (C) 2002, 2003, 2004, 2005, 2006, 2009, 2012 Vidyut Samanta and Christian Grothoff

     libextractor is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
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

#include "extractor.h"
#define gettext_noop(str) (str)

/**
 * Description for meta data categories in LE.
 */
struct MetaTypeDescription
{
  /**
   * Short (typically 1-word) description.
   */
  const char *short_description;
  
  /**
   * More detailed description.
   */
  const char *long_description;
};


/**
 * The sources of keywords as strings.
 */
static const struct MetaTypeDescription meta_type_descriptions[] = {
  /* 0 */
  { gettext_noop ("reserved"),
    gettext_noop ("reserved value, do not use") },
  { gettext_noop ("mimetype"),
    gettext_noop ("mime type") },
  { gettext_noop ("embedded filename"),
    gettext_noop ("filename that was embedded (not necessarily the current filename)") },
  { gettext_noop ("comment"),
    gettext_noop ("comment about the content") },
  { gettext_noop ("title"),
    gettext_noop ("title of the work")},
  /* 5 */
  { gettext_noop ("book title"),
    gettext_noop ("title of the book containing the work") },
  { gettext_noop ("book edition"),
    gettext_noop ("edition of the book (or book containing the work)") },
  { gettext_noop ("book chapter"),
    gettext_noop ("chapter number") },
  { gettext_noop ("journal name"),
    gettext_noop ("journal or magazine the work was published in") },
  { gettext_noop ("journal volume"),    
    gettext_noop ("volume of a journal or multi-volume book") },
  /* 10 */
  { gettext_noop ("journal number"),    
    gettext_noop ("number of a journal, magazine or tech-report") },
  { gettext_noop ("page count"),
    gettext_noop ("total number of pages of the work") },
  { gettext_noop ("page range"),
    gettext_noop ("page numbers of the publication in the respective journal or book") },
  { gettext_noop ("author name"),
    gettext_noop ("name of the author(s)") },
  { gettext_noop ("author email"),
    gettext_noop ("e-mail of the author(s)") },
  /* 15 */
  { gettext_noop ("author institution"),
    gettext_noop ("institution the author worked for") },
  { gettext_noop ("publisher"),
    gettext_noop ("name of the publisher") },
  { gettext_noop ("publisher's address"),
    gettext_noop ("Address of the publisher (often only the city)") },
  { gettext_noop ("publishing institution"),
    gettext_noop ("institution that was involved in the publishing, but not necessarily the publisher") },
  { gettext_noop ("publication series"),
    gettext_noop ("series of books the book was published in") },
  /* 20 */
  { gettext_noop ("publication type"),
    gettext_noop ("type of the tech-report") },
  { gettext_noop ("publication year"),
    gettext_noop ("year of publication (or, if unpublished, the year of creation)") },
  { gettext_noop ("publication month"),
    gettext_noop ("month of publication (or, if unpublished, the month of creation)") },
  { gettext_noop ("publication day"),
    gettext_noop ("day of publication (or, if unpublished, the day of creation), relative to the given month") },
  { gettext_noop ("publication date"),
    gettext_noop ("date of publication (or, if unpublished, the date of creation)") },
  /* 25 */
  { gettext_noop ("bibtex eprint"),
    gettext_noop ("specification of an electronic publication") },
  { gettext_noop ("bibtex entry type"),
    gettext_noop ("type of the publication for bibTeX bibliographies") },
  { gettext_noop ("language"),
    gettext_noop ("language the work uses") },
  { gettext_noop ("creation time"),
    gettext_noop ("time and date of creation") },
  { gettext_noop ("URL"),
    gettext_noop ("universal resource location (where the work is made available)") },
  /* 30 */
  { gettext_noop ("URI"),
    gettext_noop ("universal resource identifier") },
  { gettext_noop ("international standard recording code"),
    gettext_noop ("ISRC number identifying the work") },
  { gettext_noop ("MD4"),
    gettext_noop ("MD4 hash") },
  { gettext_noop ("MD5"),
    gettext_noop ("MD5 hash") },
  { gettext_noop ("SHA-0"),
    gettext_noop ("SHA-0 hash") },
  /* 35 */
  { gettext_noop ("SHA-1"), 
    gettext_noop ("SHA-1 hash") },
  { gettext_noop ("RipeMD160"),
    gettext_noop ("RipeMD150 hash") },
  { gettext_noop ("GPS latitude ref"),
    gettext_noop ("GPS latitude ref") },
  { gettext_noop ("GPS latitude"),
    gettext_noop ("GPS latitude") },
  { gettext_noop ("GPS longitude ref"),
    gettext_noop ("GPS longitude ref") },
  /* 40 */
  { gettext_noop ("GPS longitude"),
    gettext_noop ("GPS longitude") },
  { gettext_noop ("city"),
    gettext_noop ("name of the city where the document originated") },
  { gettext_noop ("sublocation"), 
    gettext_noop ("more specific location of the geographic origin") },
  { gettext_noop ("country"),
    gettext_noop ("name of the country where the document originated") },
  { gettext_noop ("country code"),
    gettext_noop ("ISO 2-letter country code for the country of origin") },
  /* 45 */
  { gettext_noop ("unknown"),
    gettext_noop ("specifics are not known") },
  { gettext_noop ("description"),
    gettext_noop ("description") },
  { gettext_noop ("copyright"),
    gettext_noop ("Name of the entity holding the copyright") },
  { gettext_noop ("rights"),
    gettext_noop ("information about rights") },
  { gettext_noop ("keywords"),
    gettext_noop ("keywords") },
  /* 50 */
  { gettext_noop ("abstract"),
    gettext_noop ("abstract") },
  { gettext_noop ("summary"),
    gettext_noop ("summary") },
  { gettext_noop ("subject"),
    gettext_noop ("subject matter") },
  { gettext_noop ("creator"),
    gettext_noop ("name of the person who created the document") },
  { gettext_noop ("format"),
    gettext_noop ("name of the document format") },
  /* 55 */
  { gettext_noop ("format version"),
    gettext_noop ("version of the document format") },
  { gettext_noop ("created by software"),
    gettext_noop ("name of the software that created the document") },
  { gettext_noop ("unknown date"),
    gettext_noop ("ambiguous date (could specify creation time, modification time or access time)") },
  { gettext_noop ("creation date"),
    gettext_noop ("date the document was created") },
  { gettext_noop ("modification date"),
    gettext_noop ("date the document was modified") },
  /* 60 */
  { gettext_noop ("last printed"),
    gettext_noop ("date the document was last printed") },
  { gettext_noop ("last saved by"),
    gettext_noop ("name of the user who saved the document last") },
  { gettext_noop ("total editing time"),
    gettext_noop ("time spent editing the document") },
  { gettext_noop ("editing cycles"),
    gettext_noop ("number of editing cycles") },
  { gettext_noop ("modified by software"),
    gettext_noop ("name of software making modifications") },
  /* 65 */
  { gettext_noop ("revision history"),
    gettext_noop ("information about the revision history") },
  { gettext_noop ("embedded file size"),
    gettext_noop ("size of the contents of the container as embedded in the file") },
  { gettext_noop ("file type"),
    gettext_noop ("standard Macintosh Finder file type information") },
  { gettext_noop ("creator"),
    gettext_noop ("standard Macintosh Finder file creator information") },
  { gettext_noop ("package name"),
    gettext_noop ("unique identifier for the package") },
  /* 70 */
  { gettext_noop ("package version"),
    gettext_noop ("version of the software and its package") },
  { gettext_noop ("section"),
    gettext_noop ("category the software package belongs to") },
  { gettext_noop ("upload priority"),
    gettext_noop ("priority for promoting the release to production") },
  { gettext_noop ("dependencies"),
    gettext_noop ("packages this package depends upon") },
  { gettext_noop ("conflicting packages"),
    gettext_noop ("packages that cannot be installed with this package") },
  /* 75 */
  { gettext_noop ("replaced packages"),
    gettext_noop ("packages made obsolete by this package") },
  { gettext_noop ("provides"),
    gettext_noop ("functionality provided by this package") },
  { gettext_noop ("recommendations"),
    gettext_noop ("packages recommended for installation in conjunction with this package") },
  { gettext_noop ("suggestions"),
    gettext_noop ("packages suggested for installation in conjunction with this package") },
  { gettext_noop ("maintainer"),
    gettext_noop ("name of the maintainer") },
  /* 80 */
  { gettext_noop ("installed size"),
    gettext_noop ("space consumption after installation") },
  { gettext_noop ("source"),
    gettext_noop ("original source code") },
  { gettext_noop ("is essential"),
    gettext_noop ("package is marked as essential") },
  { gettext_noop ("target architecture"),
    gettext_noop ("hardware architecture the contents can be used for") },
  { gettext_noop ("pre-dependency"),
    gettext_noop ("dependency that must be satisfied before installation") }, 
  /* 85 */
  { gettext_noop ("license"),
    gettext_noop ("applicable copyright license") }, 
  { gettext_noop ("distribution"),
    gettext_noop ("distribution the package is a part of") }, 
  { gettext_noop ("build host"),
    gettext_noop ("machine the package was build on") }, 
  { gettext_noop ("vendor"),
    gettext_noop ("name of the software vendor") }, 
  { gettext_noop ("target operating system"),
    gettext_noop ("operating system for which this package was made") }, 
  /* 90 */
  { gettext_noop ("software version"),
    gettext_noop ("version of the software contained in the file") }, 
  { gettext_noop ("target platform"),
    gettext_noop ("name of the architecture, operating system and distribution this package is for") }, 
  { gettext_noop ("resource type"),
    gettext_noop ("categorization of the nature of the resource that is more specific than the file format") }, 
  { gettext_noop ("library search path"),
    gettext_noop ("path in the file system to be considered when looking for required libraries") }, 
  { gettext_noop ("library dependency"),
    gettext_noop ("name of a library that this file depends on") }, 
  /* 95 */
  { gettext_noop ("camera make"),
    gettext_noop ("camera make") }, 
  { gettext_noop ("camera model"),
    gettext_noop ("camera model") }, 
  { gettext_noop ("exposure"),
    gettext_noop ("exposure") }, 
  { gettext_noop ("aperture"),
    gettext_noop ("aperture") }, 
  { gettext_noop ("exposure bias"),
    gettext_noop ("exposure bias") }, 
  /* 100 */
  { gettext_noop ("flash"),
    gettext_noop ("flash") }, 
  { gettext_noop ("flash bias"),
    gettext_noop ("flash bias") }, 
  { gettext_noop ("focal length"),
    gettext_noop ("focal length") }, 
  { gettext_noop ("focal length 35mm"),
    gettext_noop ("focal length 35mm") }, 
  { gettext_noop ("iso speed"),
    gettext_noop ("iso speed") }, 
  /* 105 */
  { gettext_noop ("exposure mode"),
    gettext_noop ("exposure mode") }, 
  { gettext_noop ("metering mode"),
    gettext_noop ("metering mode") }, 
  { gettext_noop ("macro mode"),
    gettext_noop ("macro mode") }, 
  { gettext_noop ("image quality"),
    gettext_noop ("image quality") }, 
  { gettext_noop ("white balance"),
    gettext_noop ("white balance") }, 
  /* 110 */
  { gettext_noop ("orientation"),
    gettext_noop ("orientation") }, 
  { gettext_noop ("magnification"),
    gettext_noop ("magnification") }, 
  { gettext_noop ("image dimensions"),
    gettext_noop ("size of the image in pixels (width times height)") }, 
  { gettext_noop ("produced by software"),
    gettext_noop ("produced by software") }, /* what is the exact difference between the software
			    creator and the software producer? PDF and DVI
			    both have this distinction (i.e., Writer vs.
			    OpenOffice) */
  { gettext_noop ("thumbnail"),
    gettext_noop ("smaller version of the image for previewing") }, 
  /* 115 */
  { gettext_noop ("image resolution"),
    gettext_noop ("resolution in dots per inch") }, 
  { gettext_noop ("source"),
    gettext_noop ("Originating entity") }, 
  { gettext_noop ("character set"),
    gettext_noop ("character encoding used") }, 
  { gettext_noop ("line count"),
    gettext_noop ("number of lines") }, 
  { gettext_noop ("paragraph count"),
    gettext_noop ("number of paragraphs") }, 
  /* 120 */
  { gettext_noop ("word count"),
    gettext_noop ("number of words") }, 
  { gettext_noop ("character count"),
    gettext_noop ("number of characters") }, 
  { gettext_noop ("page orientation"),
    gettext_noop ("page orientation") }, 
  { gettext_noop ("paper size"),
    gettext_noop ("paper size") }, 
  { gettext_noop ("template"),
    gettext_noop ("template the document uses or is based on") }, 
  /* 125 */
  { gettext_noop ("company"),
    gettext_noop ("company") }, 
  { gettext_noop ("manager"),
    gettext_noop ("manager") }, 
  { gettext_noop ("revision number"),
    gettext_noop ("revision number") }, 
  { gettext_noop ("duration"),
    gettext_noop ("play time for the medium") }, 
  { gettext_noop ("album"),
    gettext_noop ("name of the album") }, 
  /* 130 */
  { gettext_noop ("artist"),
    gettext_noop ("name of the artist or band") }, 
  { gettext_noop ("genre"),
    gettext_noop ("genre") }, 
  { gettext_noop ("track number"),
    gettext_noop ("original number of the track on the distribution medium") }, 
  { gettext_noop ("disk number"),
    gettext_noop ("number of the disk in a multi-disk (or volume) distribution") }, 
  { gettext_noop ("performer"),
    gettext_noop ("The artist(s) who performed the work (conductor, orchestra, soloists, actor, etc.)") }, 
  /* 135 */
  { gettext_noop ("contact"),
    gettext_noop ("Contact information for the creator or distributor") }, 
  { gettext_noop ("song version"),
    gettext_noop ("name of the version of the song (i.e. remix information)") }, 
  { gettext_noop ("picture"),
    gettext_noop ("associated misc. picture") }, 
  { gettext_noop ("cover picture"),
    gettext_noop ("picture of the cover of the distribution medium") }, 
  { gettext_noop ("contributor picture"),
    gettext_noop ("picture of one of the contributors") }, 
  /* 140 */
  { gettext_noop ("event picture"),
    gettext_noop ("picture of an associated event") }, 
  { gettext_noop ("logo"),
    gettext_noop ("logo of an associated organization") }, 
  { gettext_noop ("broadcast television system"),
    gettext_noop ("name of the television system for which the data is coded") }, 
  { gettext_noop ("source device"),
    gettext_noop ("device used to create the object") }, 
  { gettext_noop ("disclaimer"),
    gettext_noop ("legal disclaimer") }, 
  /* 145 */
  { gettext_noop ("warning"),
    gettext_noop ("warning about the nature of the content") }, 
  { gettext_noop ("page order"),
    gettext_noop ("order of the pages") }, 
  { gettext_noop ("writer"),
    gettext_noop ("contributing writer") }, 
  { gettext_noop ("product version"),
    gettext_noop ("product version") }, 
  { gettext_noop ("contributor"),
    gettext_noop ("name of a contributor") }, 
  /* 150 */
  { gettext_noop ("movie director"),
    gettext_noop ("name of the director") }, 
  { gettext_noop ("network"),
    gettext_noop ("name of the broadcasting network or station") }, 
  { gettext_noop ("show"),
    gettext_noop ("name of the show") }, 
  { gettext_noop ("chapter name"),
    gettext_noop ("name of the chapter") }, 
  { gettext_noop ("song count"),
    gettext_noop ("number of songs") }, 
  /* 155 */
  { gettext_noop ("starting song"),
    gettext_noop ("number of the first song to play") }, 
  { gettext_noop ("play counter"),
    gettext_noop ("number of times the media has been played") }, 
  { gettext_noop ("conductor"),
    gettext_noop ("name of the conductor") }, 
  { gettext_noop ("interpretation"),
    gettext_noop ("information about the people behind interpretations of an existing piece") }, 
  { gettext_noop ("composer"),
    gettext_noop ("name of the composer") }, 
  /* 160 */
  { gettext_noop ("beats per minute"),
    gettext_noop ("beats per minute") }, 
  { gettext_noop ("encoded by"),
    gettext_noop ("name of person or organization that encoded the file") }, 
  { gettext_noop ("original title"),
    gettext_noop ("title of the original work") }, 
  { gettext_noop ("original artist"),
    gettext_noop ("name of the original artist") }, 
  { gettext_noop ("original writer"),
    gettext_noop ("name of the original lyricist or writer") }, 
  /* 165 */
  { gettext_noop ("original release year"),
    gettext_noop ("year of the original release") }, 
  { gettext_noop ("original performer"),
    gettext_noop ("name of the original performer") }, 
  { gettext_noop ("lyrics"),
    gettext_noop ("lyrics of the song or text description of vocal activities") }, 
  { gettext_noop ("popularity"),
    gettext_noop ("information about the file's popularity") }, 
  { gettext_noop ("licensee"),
    gettext_noop ("name of the owner or licensee of the file") }, 
  /* 170 */
  { gettext_noop ("musician credit list"),
    gettext_noop ("names of contributing musicians") }, 
  { gettext_noop ("mood"),
    gettext_noop ("keywords reflecting the mood of the piece") }, 
  { gettext_noop ("subtitle"),
    gettext_noop ("subtitle of this part") }, 
  { gettext_noop ("display type"),
    gettext_noop ("what rendering method should be used to display this item") }, 
  { gettext_noop ("full data"),
    gettext_noop ("entry that contains the full, original binary data (not really meta data)") }, 
  /* 175 */
  { gettext_noop ("rating"),
    gettext_noop ("rating of the content") },
  { gettext_noop ("organization"),
    gettext_noop ("organization") },
  { gettext_noop ("ripper"), /* any difference to "encoded by"? */
    gettext_noop ("ripper") },
  { gettext_noop ("producer"), 
    gettext_noop ("producer") },
  { gettext_noop ("group"), 
    gettext_noop ("name of the group or band") },
  /* 180 */
  { gettext_noop ("original filename"),
    gettext_noop ("name of the original file (reserved for GNUnet)") },
  { gettext_noop ("disc count"),
    gettext_noop ("count of discs inside collection this disc belongs to") },
  { gettext_noop ("codec"),
    gettext_noop ("codec the data is stored in") },
  { gettext_noop ("video codec"),
    gettext_noop ("codec the video data is stored in") },
  { gettext_noop ("audio codec"),
    gettext_noop ("codec the audio data is stored in") },
  /* 185 */
  { gettext_noop ("subtitle codec"),
    gettext_noop ("codec/format the subtitle data is stored in") },
  { gettext_noop ("container format"),
    gettext_noop ("container format the data is stored in") },
  { gettext_noop ("bitrate"),
    gettext_noop ("exact or average bitrate in bits/s") },
  { gettext_noop ("nominal bitrate"),
    gettext_noop ("nominal bitrate in bits/s. The actual bitrate might be different from this target bitrate.") },
  { gettext_noop ("minimum bitrate"),
    gettext_noop ("minimum bitrate in bits/s") },
  /* 190 */
  { gettext_noop ("maximum bitrate"),
    gettext_noop ("maximum bitrate in bits/s") },
  { gettext_noop ("serial"),
    gettext_noop ("serial number of track") },
  { gettext_noop ("encoder"),
    gettext_noop ("encoder used to encode this stream") },
  { gettext_noop ("encoder version"),
    gettext_noop ("version of the encoder used to encode this stream") },
  { gettext_noop ("track gain"),
    gettext_noop ("track gain in db") },
  /* 195 */
  { gettext_noop ("track peak"),
    gettext_noop ("peak of the track") },
  { gettext_noop ("album gain"),
    gettext_noop ("album gain in db") },
  { gettext_noop ("album peak"),
    gettext_noop ("peak of the album") },
  { gettext_noop ("reference level"),
    gettext_noop ("reference level of track and album gain values") },
  { gettext_noop ("location name"),
    gettext_noop ("human readable descriptive location of where the media has been recorded or produced") },
  /* 200 */
  { gettext_noop ("location elevation"),
    gettext_noop ("geo elevation of where the media has been recorded or produced in meters according to WGS84 (zero is average sea level)") },
  { gettext_noop ("location horizontal error"),
    gettext_noop ("represents the expected error on the horizontal positioning in meters") },
  { gettext_noop ("location movement speed"),
    gettext_noop ("speed of the capturing device when performing the capture. Represented in m/s") },
  { gettext_noop ("location movement direction"),
    gettext_noop ("indicates the movement direction of the device performing the capture of a media. It is represented as degrees in floating point representation, 0 means the geographic north, and increases clockwise") },
  { gettext_noop ("location capture direction"),
    gettext_noop ("indicates the direction the device is pointing to when capturing a media. It is represented as degrees in floating point representation, 0 means the geographic north, and increases clockwise") },
  /* 205 */
  { gettext_noop ("show episode number"),
    gettext_noop ("number of the episode within a season/show") },
  { gettext_noop ("show season number"),
    gettext_noop ("number of the season of a show/series") },
  { gettext_noop ("grouping"),
    gettext_noop ("groups together media that are related and spans multiple tracks. An example are multiple pieces of a concerto") },
  { gettext_noop ("device manufacturer"),
    gettext_noop ("manufacturer of the device used to create the media") },
  { gettext_noop ("device model"),
    gettext_noop ("model of the device used to create the media") },
  /* 210 */
  { gettext_noop ("audio language"),
    gettext_noop ("language of the audio track") },
  { gettext_noop ("channels"),
    gettext_noop ("number of audio channels") },
  { gettext_noop ("sample rate"),
    gettext_noop ("sample rate of the audio track") },
  { gettext_noop ("audio depth"),
    gettext_noop ("number of bits per audio sample") },
  { gettext_noop ("audio bitrate"),
    gettext_noop ("bitrate of the audio track") },
  /* 215 */
  { gettext_noop ("maximum audio bitrate"),
    gettext_noop ("maximum audio bitrate") },
  { gettext_noop ("video dimensions"),
    gettext_noop ("width and height of the video track (WxH)") },
  { gettext_noop ("video depth"),
    gettext_noop ("numbers of bits per pixel") },
  { gettext_noop ("frame rate"),
    gettext_noop ("number of frames per second (as D/N or floating point)") },
  { gettext_noop ("pixel aspect ratio"),
    gettext_noop ("pixel aspect ratio (as D/N)") },
  /* 220 */
  { gettext_noop ("video bitrate"),
    gettext_noop ("video bitrate") },
  { gettext_noop ("maximum video bitrate"),
    gettext_noop ("maximum video bitrate") },
  { gettext_noop ("subtitle language"),
    gettext_noop ("language of the subtitle track") },
  { gettext_noop ("video language"),
    gettext_noop ("language of the video track") },
  { gettext_noop ("table of contents"),
    gettext_noop ("chapters, contents or bookmarks (in xml format)") },
  /* 225 */
  { gettext_noop ("video duration"),
    gettext_noop ("duration of a video stream") },
  { gettext_noop ("audio duration"),
    gettext_noop ("duration of an audio stream") },
  { gettext_noop ("subtitle duration"),
    gettext_noop ("duration of a subtitle stream") },

  { gettext_noop ("last"),
    gettext_noop ("last") }
};

/**
 * Total number of keyword types (for bounds-checking) 
 */
#define HIGHEST_METATYPE_NUMBER (sizeof (meta_type_descriptions) / sizeof(*meta_type_descriptions))


/**
 * Get the textual name of the keyword.
 *
 * @param type meta type to get a UTF-8 string for
 * @return NULL if the type is not known, otherwise
 *         an English (locale: C) string describing the type;
 *         translate using 'dgettext ("libextractor", rval)'
 */
const char *
EXTRACTOR_metatype_to_string (enum EXTRACTOR_MetaType type)
{
  if ( (type < 0) || (type >= HIGHEST_METATYPE_NUMBER) )
    return NULL;
  return meta_type_descriptions[type].short_description;
}


/**
 * Get a long description for the meta type.
 *
 * @param type meta type to get a UTF-8 description for
 * @return NULL if the type is not known, otherwise
 *         an English (locale: C) string describing the type;
 *         translate using 'dgettext ("libextractor", rval)'
 */
const char *
EXTRACTOR_metatype_to_description (enum EXTRACTOR_MetaType type)
{
  if ( (type < 0) || (type >= HIGHEST_METATYPE_NUMBER) )
    return NULL;
  return meta_type_descriptions[type].long_description;
}


/**
 * Return the highest type number, exclusive as in [0,max).
 *
 * @return highest legal metatype number for this version of libextractor
 */
enum EXTRACTOR_MetaType
EXTRACTOR_metatype_get_max ()
{
  return HIGHEST_METATYPE_NUMBER;
}


/* end of extractor_metatypes.c */
