/*
 * scheduler.c - gnunet-web version of util/scheduler.c
 * Copyright (C) 2015  David Barksdale <amatus@amatus.name>
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
#include "gnunet_util_lib.h"

extern struct GNUNET_SCHEDULER_Task *
GNUNET_SCHEDULER_add_delayed_with_priority_js (double delay,
    enum GNUNET_SCHEDULER_Priority priority,
    GNUNET_SCHEDULER_TaskCallback task,
    void *task_cls);

struct GNUNET_SCHEDULER_Task *
GNUNET_SCHEDULER_add_delayed_with_priority (struct GNUNET_TIME_Relative delay,
    enum GNUNET_SCHEDULER_Priority priority,
    GNUNET_SCHEDULER_TaskCallback task,
    void *task_cls)
{
  return GNUNET_SCHEDULER_add_delayed_with_priority_js(
      (double)(delay.rel_value_us / 1000), priority, task, task_cls);
}

struct GNUNET_SCHEDULER_Task *
GNUNET_SCHEDULER_add_with_priority (enum GNUNET_SCHEDULER_Priority prio,
    GNUNET_SCHEDULER_TaskCallback task,
    void *task_cls)
{
  return GNUNET_SCHEDULER_add_delayed_with_priority (GNUNET_TIME_UNIT_ZERO,
      prio, task, task_cls);
}

struct GNUNET_SCHEDULER_Task *
GNUNET_SCHEDULER_add_delayed (struct GNUNET_TIME_Relative delay,
    GNUNET_SCHEDULER_TaskCallback task, void *task_cls)
{
  return GNUNET_SCHEDULER_add_delayed_with_priority (delay,
      GNUNET_SCHEDULER_PRIORITY_DEFAULT, task, task_cls);
}

struct GNUNET_SCHEDULER_Task *
GNUNET_SCHEDULER_add_now (GNUNET_SCHEDULER_TaskCallback task, void *task_cls)
{
  return GNUNET_SCHEDULER_add_delayed (GNUNET_TIME_UNIT_ZERO, task, task_cls);
}

void
GNUNET_SCHEDULER_add_continuation (GNUNET_SCHEDULER_TaskCallback task,
    void *task_cls,
    enum GNUNET_SCHEDULER_Reason reason)
{
  GNUNET_SCHEDULER_add_now(task, task_cls);
}

void
GNUNET_SCHEDULER_run_task (GNUNET_SCHEDULER_TaskCallback task, void *task_cls)
{
  struct GNUNET_SCHEDULER_TaskContext tc;

  tc.reason = GNUNET_SCHEDULER_REASON_TIMEOUT;
  tc.read_ready = NULL;
  tc.write_ready = NULL;
  task (task_cls, &tc);
}

/* vim: set expandtab ts=2 sw=2: */
