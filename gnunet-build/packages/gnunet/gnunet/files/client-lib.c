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

void *
GNUNET_FS_ProgressInfo_get_download_cctx(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.cctx;
}

double
GNUNET_FS_ProgressInfo_get_download_size(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.size;
}

double
GNUNET_FS_ProgressInfo_get_download_completed(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.completed;
}

const void *
GNUNET_FS_ProgressInfo_get_download_progress_data(
    struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.specifics.progress.data;
}

double
GNUNET_FS_ProgressInfo_get_download_progress_offset(
    struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.specifics.progress.offset;
}

double
GNUNET_FS_ProgressInfo_get_download_progress_data_len(
    struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.specifics.progress.data_len;
}

unsigned int
GNUNET_FS_ProgressInfo_get_download_progress_depth(
    struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.download.specifics.progress.depth;
}

void *
GNUNET_FS_ProgressInfo_get_search_cctx(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.search.cctx;
}

const struct GNUNET_CONTAINER_MetaData *
GNUNET_FS_ProgressInfo_get_search_result_meta(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.search.specifics.result.meta;
}

const struct GNUNET_FS_Uri *
GNUNET_FS_ProgressInfo_get_search_result_uri(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->value.search.specifics.result.uri;
}

enum GNUNET_FS_Status
GNUNET_FS_ProgressInfo_get_status(struct GNUNET_FS_ProgressInfo *pi)
{
  return pi->status;
}

struct GNUNET_FS_BlockOptions *
GNUNET_FS_BlockOptions_new(double expiration_time, uint32_t anonymity_level,
    uint32_t content_priority, uint32_t replication_level)
{
  struct GNUNET_FS_BlockOptions *bo;

  bo = malloc(sizeof *bo);
  if (bo) {
    bo->expiration_time.abs_value_us = expiration_time;
    bo->anonymity_level = anonymity_level;
    bo->content_priority = content_priority;
    bo->replication_level = replication_level;
  }
  return bo;
}

double
GNUNET_FS_uri_chk_get_file_size2(const struct GNUNET_FS_Uri *uri)
{
  return GNUNET_FS_uri_chk_get_file_size(uri);
}

struct GNUNET_FS_DownloadContext *
GNUNET_FS_download_start_simple(struct GNUNET_FS_Handle *h,
    const struct GNUNET_FS_Uri *uri, uint32_t anonymity, void *cctx)
{
  uint64_t length = GNUNET_FS_uri_chk_get_file_size(uri);

  return GNUNET_FS_download_start(h, uri, NULL, NULL, NULL, 0, length,
      anonymity, 0, cctx, NULL);
}

/* vim: set expandtab ts=2 sw=2: */
