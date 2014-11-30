/*
 * client-lib.c - linked into gnunet-web client library
 * Copyright (C) 2014  David Barksdale <amatus@amatus.name>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"
#include "gnunet_fs_service.h"

void *
GNUNET_FS_ProgressInfo_get_publish_cctx(struct GNUNET_FS_ProgressInfo *pi)
{
	return pi->value.publish.cctx;
}

double
GNUNET_FS_ProgressInfo_get_publish_size(struct GNUNET_FS_ProgressInfo *pi)
{
	return pi->value.publish.size;
}

double
GNUNET_FS_ProgressInfo_get_publish_completed(struct GNUNET_FS_ProgressInfo *pi)
{       
        return pi->value.publish.completed;
}

const struct GNUNET_FS_Uri *
GNUNET_FS_ProgressInfo_get_publish_completed_chk_uri(
		struct GNUNET_FS_ProgressInfo *pi)
{
	return pi->value.publish.specifics.completed.chk_uri;
}
