/*
 * ProviewR   Open Source Process Control.
 * Copyright (C) 2005-2019 SSAB EMEA AB.
 *
 * This file is part of ProviewR.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ProviewR. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking ProviewR statically or dynamically with other modules is
 * making a combined work based on ProviewR. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * ProviewR give you permission to, from the build function in the
 * ProviewR Configurator, combine ProviewR with modules generated by the
 * ProviewR PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every
 * copy of the combined work is accompanied by a complete copy of
 * the source code of ProviewR (the version used to produce the
 * combined work), being distributed under the terms of the GNU
 * General Public License plus this exception.
 */

/* rt_io_m_rack_ssab.c -- io methods for ssab rack objects. */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "pwr_basecomponentclasses.h"
#include "pwr_ssaboxclasses.h"
#include "rt_io_base.h"
#include "rt_io_rack_init.h"
#include "rt_io_m_ssab_locals.h"
#include "rt_io_msg.h"

/*----------------------------------------------------------------------------*\

\*----------------------------------------------------------------------------*/

static pwr_tStatus IoRackInit(io_tCtx ctx, io_sAgent* ap, io_sRack* rp)
{
  io_sRackLocal* local;

  /* Open Qbus driver */
  local = calloc(1, sizeof(*local));
  rp->Local = local;

  local->Qbus_fp = open("/dev/qbus", O_RDWR);
  if (local->Qbus_fp == -1) {
    errh_Error("Qbus initialization error, IO rack %s", rp->Name);
    ctx->Node->EmergBreakTrue = 1;
    return IO__ERRDEVICE;
  }

  errh_Info("Init of IO rack %s", rp->Name);
  return 1;
}

static pwr_tStatus IoRackSwap(
    io_tCtx ctx, io_sAgent* ap, io_sRack* rp, io_eEvent event

    )
{
  io_sRackLocal* local;

  switch (event) {
  case io_eEvent_IoCommSwapInit:
  case io_eEvent_IoCommSwap:
    if (!rp->Local) {
      /* Open Qbus driver */
      local = calloc(1, sizeof(*local));
      rp->Local = local;

      local->Qbus_fp = open("/dev/qbus", O_RDWR);
      if (local->Qbus_fp == -1) {
        errh_Error("Qbus swap initialization error, IO rack %s", rp->Name);
        return IO__ERRDEVICE;
      }

      errh_Info("Swap init of IO rack %s", rp->Name);
    }
  default:;
  }

  return 1;
}

static pwr_tStatus IoRackClose(io_tCtx ctx, io_sAgent* ap, io_sRack* rp)
{
  io_sRackLocal* local;

  /* Close Qbus driver */
  local = rp->Local;

  close(local->Qbus_fp);
  free((char*)local);

  return 1;
}

/*----------------------------------------------------------------------------*\

\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Rack_SSAB) = { pwr_BindIoMethod(IoRackInit),
  pwr_BindIoMethod(IoRackSwap), pwr_BindIoMethod(IoRackClose), pwr_NullMethod };