/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2017 SSAB EMEA AB.
 *
 * This file is part of Proview.
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
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 */

/* rt_redu.c -- Redundancy */

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_names.h"
#include "pwr_systemclasses.h"
#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"
#include "rt_gdh.h"
#include "rt_qdb.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"
#include "rt_redu.h"
#include "rt_redu_msg.h"
#include "co_timelog.h"

static pwr_tStatus add_table_object( redu_tCtx ctx, pwr_tAttrRef *o);
static pwr_tStatus add_table_attr( redu_tCtx ctx, pwr_tAttrRef *aref, gdh_sAttrDef *bd);

pwr_tStatus redu_create_table( redu_tCtx ctx)
{
  pwr_tStatus sts;
  pwr_tAttrRef aref;
  unsigned int i;

  if ( ctx->t)
    redu_free_table( ctx);

  for ( i = 0; i < sizeof(ctx->packetp->Hierarchies)/sizeof(ctx->packetp->Hierarchies[0]); i++) {
    if ( ctx->packetp->Hierarchies[i].vid == 0)
      continue;

    aref = cdh_ObjidToAref( ctx->packetp->Hierarchies[i]);

    sts = add_table_object( ctx, &aref);
    if ( EVEN(sts)) return sts;
  }
#if 0
  /* Loop trough all objects and pick out attributes that should be transfered */
  for ( sts = gdh_GetRootList( &oid); ODD(sts); sts = gdh_GetNextSibling( oid, &oid)) {
    aref = cdh_ObjidToAref( oid);
    
    sts = add_table_object( ctx, &aref);
    if ( EVEN(sts)) return sts;
  }
#endif

  ctx->table_version = ctx->nodep->CurrentVersion;

  return REDU__TABLECREATED;
}

void redu_free( redu_tCtx ctx)
{
  if ( ctx->t)
    redu_free_table( ctx);
  free( ctx);
}

void redu_free_table( redu_tCtx ctx)
{
  redu_sTable *e, *enext;

  e = ctx->t;
  while ( e) {
    enext = e->next;
    free( e);
    e = enext;
  }
  ctx->t = 0;
  ctx->t_last = 0;
  ctx->attr_cnt = 0;
  ctx->current_offset = 0;
  ctx->table_version = pwr_cNTime;
}
    
static pwr_tStatus add_table_object( redu_tCtx ctx, pwr_tAttrRef *o) 
{
  pwr_tStatus sts;
  pwr_tTid tid;
  int elements;
  pwr_tAttrRef maref;
  pwr_tOName aname;
  pwr_tAttrRef aref;
  int i, j;
  int rows;
  gdh_sAttrDef *bd;

  // Test 
  //pwr_tAName name;
  //sts = gdh_AttrrefToName( o, name, sizeof(name), cdh_mNName);
  //printf( "%s\n", name);

  sts = gdh_GetAttrRefTid( o, &tid);
  if ( EVEN(sts)) return sts;

  if ( tid == pwr_cClass_MountObject) {
    /* Skip for now... TODO */
    pwr_sClass_MountObject *p;
    pwr_tOid oid;
    gdh_sVolumeInfo info;

    sts = gdh_AttrRefToPointer( o, (void **)&p);
    if ( EVEN(sts)) return sts;

    oid = p->Object;

    sts = gdh_GetVolumeInfo( oid.vid, &info);
    if ( EVEN(sts)) return sts;
    
    if ( info.cid == pwr_cClass_SharedVolume ||
	 info.cid == pwr_cClass_SubVolume) {
      maref = cdh_ObjidToAref( oid);
      o = &maref;
    }
    else
      return REDU__SUCCESS;

    sts = gdh_GetAttrRefTid( o, &tid);
    if ( EVEN(sts)) return sts;
  }

  sts = gdh_GetObjectBodyDef( tid, &bd, &rows, pwr_cNOid);
  if ( EVEN(sts)) return sts;

  for ( i = 0; i < rows; i++) {
    if ( bd[i].attr->Param.Info.Flags & PWR_MASK_RTVIRTUAL || 
	 bd[i].attr->Param.Info.Flags & PWR_MASK_PRIVATE)
      continue;
    if ( !(bd[i].attr->Param.Info.Flags & PWR_MASK_CLASS) &&
	 !(bd[i].attr->Param.Info.Flags & PWR_MASK_REDUTRANSFER))	 
      continue;

    if ( bd[i].attr->Param.Info.Flags & PWR_MASK_ARRAY)
      elements = bd[i].attr->Param.Info.Elements;
    else
      elements = 1;

    if ( bd[i].attr->Param.Info.Flags & PWR_MASK_CLASS) {
      if ( elements == 1) {
	if ( strncmp( bd[i].attrName, "Super.", 6) == 0)
	  strcpy( aname, &bd[i].attrName[6]);
	else
	  strcpy( aname, bd[i].attrName);

	sts = gdh_ArefANameToAref( o, aname, &aref);
	if ( EVEN(sts)) return sts;

	sts = add_table_object( ctx, &aref);
	if ( EVEN(sts)) return sts;
      }
      else {
	for ( j = 0; j < elements; j++) {
	  if ( strncmp( bd[i].attrName, "Super.", 6) == 0)
	    sprintf( aname, "%s[%d]", &bd[i].attrName[6], j);
	  else
	    sprintf( aname, "%s[%d]", bd[i].attrName, j);

	  sts = gdh_ArefANameToAref( o, aname, &aref);
	  if ( EVEN(sts)) return sts;

	  sts = add_table_object( ctx, &aref);
	  if ( EVEN(sts)) return sts;
	}
      }
    }
    else {
      if ( strncmp( bd[i].attrName, "Super.", 6) == 0)
	strcpy( aname, &bd[i].attrName[6]);
      else
	strcpy( aname, bd[i].attrName);

      sts = gdh_ArefANameToAref( o, aname, &aref);
      if ( EVEN(sts)) return sts;

      sts = add_table_attr( ctx, &aref, &bd[i]);
    }
  }

  /* Add children */
  if ( o->Flags.b.Object) {
    pwr_tObjid coid;

    for ( sts = gdh_GetChild( o->Objid, &coid); ODD(sts); sts = gdh_GetNextSibling( coid, &coid)) {
      aref = cdh_ObjidToAref( coid);

      sts = add_table_object( ctx, &aref);
      if ( EVEN(sts)) return sts;      
    }
  }
  return REDU__SUCCESS;
}

static pwr_tStatus add_table_attr( redu_tCtx ctx, pwr_tAttrRef *aref, gdh_sAttrDef *bd)
{
  pwr_tStatus sts;
  redu_sTable *t;
  void *p;

  sts = gdh_AttrRefToPointer( aref, &p);
  if ( EVEN(sts)) return sts;

  if ( aref->Flags.b.Indirect) {
    if ( *(unsigned long *)p == 0)
      return REDU__SUCCESS;
    p = gdh_TranslateRtdbPointer( *(unsigned long *)p);
  }

  t = (redu_sTable *)calloc( 1, sizeof(*t));

  if ( !ctx->t)
    ctx->t = t;
  else 
    ctx->t_last->next = t;
  ctx->t_last = t;
  
  t->aref = *aref;
  t->offset = ctx->current_offset;
  t->size = bd->attr->Param.Info.Size;
  t->p = p;

  ctx->current_offset += t->size;
  ctx->attr_cnt++;
  return REDU__SUCCESS;
}

pwr_tStatus redu_create_message( redu_tCtx ctx, void **msg)
{
  char *buf;
  char *bufp;
  redu_sTable *e;
  pwr_tTime start_time;
  pwr_tTime end_time;
  pwr_tDeltaTime dtime;

  buf = malloc( sizeof(redu_sMsgHeader) + ctx->current_offset);

  
  ((redu_sMsgHeader *)buf)->h.type = redu_eMsgType_Cyclic;
  ((redu_sMsgHeader *)buf)->size = ctx->current_offset;  
  ((redu_sMsgHeader *)buf)->version = ctx->table_version;

  time_GetTime( &start_time);
  bufp = buf + sizeof(redu_sMsgHeader);

  for ( e = ctx->t; e; e = e->next) {
    memcpy( bufp, e->p, e->size);
    bufp += e->size;
  }

  time_GetTime( &end_time);
  time_Adiff( &dtime, &end_time, &start_time);
  time_DToFloat( &ctx->msg_time, &dtime);
  if ( ctx->packetp) {
    ctx->packetp->TransmitCnt++;
    ctx->packetp->PackTime = ctx->msg_time;
  }
  *msg = buf;
  return REDU__SUCCESS;
}

pwr_tStatus redu_unpack_message( redu_tCtx ctx, void *msg)
{
  char *bufp;
  redu_sTable *e;
  pwr_tTime start_time;
  pwr_tTime end_time;
  pwr_tDeltaTime dtime;
  int tsts;
  
  tsts = time_Acomp_NE( &ctx->table_version, &((redu_sMsgHeader *)msg)->version);
  if ( tsts != 0) {
    if ( tsts == -2)
      return 0;
    else
      // Request new table...
      return 0;
  }

  time_GetTime( &start_time);
  bufp = msg + sizeof(redu_sMsgHeader);

  for ( e = ctx->t; e; e = e->next) {
    memcpy( e->p, bufp, e->size);
    bufp += e->size;
  }

  time_GetTime( &end_time);
  time_Adiff( &dtime, &end_time, &start_time);
  time_DToFloat( &ctx->msg_time, &dtime);
  if ( ctx->packetp) {
    ctx->packetp->ReceiveCnt++;
    ctx->packetp->UnpackTime = ctx->msg_time;
  }
  return REDU__SUCCESS;
}

pwr_tStatus redu_send_table( redu_tCtx ctx, void **table_msg)
{
  char *msg;
  char *msgp;
  int size;
  redu_sTable *e;

  size = ctx->attr_cnt * ( sizeof(pwr_tAttrRef) + sizeof(pwr_tUInt32));
  msg = malloc( sizeof(redu_sTableMsgHeader) + size);
  ((redu_sTableMsgHeader *)msg)->h.type = redu_eMsgType_Table;
  ((redu_sTableMsgHeader *)msg)->attributes = ctx->attr_cnt;
  ((redu_sTableMsgHeader *)msg)->size = size;
  ((redu_sTableMsgHeader *)msg)->version = ctx->table_version;

  msgp = msg + sizeof(redu_sTableMsgHeader);
  for ( e = ctx->t; e; e = e->next) {
    memcpy( &((redu_sTableMsgElement *)msgp)->aref, &e->aref, sizeof(pwr_tAttrRef));
    ((redu_sTableMsgElement *)msgp)->size = e->size;
    msgp += sizeof(redu_sTableMsgElement);
  }
  
  if ( ctx->packetp) {
    ctx->packetp->TablePacketSize = size;
    ctx->packetp->Attributes = ctx->attr_cnt;
  }
  *table_msg = msg;
  return REDU__TABLESENT;
}

pwr_tStatus redu_receive_table( redu_tCtx ctx, void *table_msg)
{
  char *msg;
  char *msgp;
  int attributes;
  int i;
  redu_sTable *e;
  pwr_tStatus sts;
  int tsts;
  void *p;

  msg = table_msg;

  if ( ctx->t)
    redu_free_table( ctx);

  timelog( 2, "Table received");
  tsts = time_Acomp_NE( &ctx->nodep->CurrentVersion, &((redu_sTableMsgHeader *)msg)->version);
  if ( tsts != 0) {
    if ( tsts == -2)
      return 0;
    else
      errh_Warning( "Table version differs from current load version");
  }

  attributes = ((redu_sTableMsgHeader *)msg)->attributes;

  msgp = msg + sizeof(redu_sTableMsgHeader);
  for ( i = 0; i < attributes; i++) {
    e = (redu_sTable *)calloc( 1, sizeof(*e));    
    memcpy( &e->aref, &((redu_sTableMsgElement *)msgp)->aref, sizeof(pwr_tAttrRef));
    e->size = ((redu_sTableMsgElement *)msgp)->size;
    if ( ctx->t_last)
      e->offset = ctx->t_last->offset + ctx->t_last->size;
    else
      e->offset = 0;
    sts = gdh_AttrRefToPointer( &e->aref, &p);
    if ( EVEN(sts)) return sts;

    if ( e->aref.Flags.b.Indirect)
      p = gdh_TranslateRtdbPointer( *(unsigned long *)p);

    e->p = p;

    if ( !ctx->t)
      ctx->t = e;
    else
      ctx->t_last->next = e;
    ctx->t_last = e;
    ctx->current_offset += e->size;
    ctx->attr_cnt++;
    msgp += sizeof(redu_sTableMsgElement);    
  }

  ctx->table_version = ((redu_sTableMsgHeader *)msg)->version;

  return REDU__TABLERECEIVED;
}

void redu_print_table( redu_tCtx ctx)
{
  redu_sTable *e;
  pwr_tAName name;
  pwr_tStatus sts;

  e = ctx->t;
  while ( e) {
    sts = gdh_AttrrefToName( &e->aref, name, sizeof(name), cdh_mNName);
    printf( "%5d %5d %s\n", e->offset, e->size, name);

    e = e->next;
  }
}

pwr_tStatus redu_send( redu_tCtx ctx, void *msg, int size, unsigned int msg_id)
{  
  pwr_tStatus sts;
  qcom_sPut put;

  put.data = msg;
  put.size = size;
  put.msg_id = msg_id;
  put.prio = ctx->prio;   
  put.type.b = qcom_eBtype__;
  put.type.s = qcom_eStype__;
  put.reply.qix = 0;
  put.reply.nid = 0;
  put.allocate = 1;
  
  qcom_Put(&sts, &ctx->send_qid, &put);
  return sts;
}

pwr_tStatus redu_init( redu_tCtx *ctx, pwr_sNode *nodep, pwr_sClass_RedcomPacket *packetp)
{
  qcom_sQattr 	qattr;
  pwr_tStatus sts;
  redu_tCtx c;

  c = (redu_tCtx) calloc( 1, sizeof(redu_sCtx));
  *ctx = c;
  c->nodep = nodep;
  c->packetp = packetp;
  c->prio = packetp->Prio;
  c->msgid_table = 1000 + c->prio;
  c->msgid_cyclic = c->prio;
  
  switch ( c->prio) {
  case redu_ePrio_1:
    c->rcv_qid.qix = redu_cQixPrio1;
    break;
  case redu_ePrio_2:
    c->rcv_qid.qix = redu_cQixPrio2;
    break;
  case redu_ePrio_3:
    c->rcv_qid.qix = redu_cQixPrio3;
    break;
  case redu_ePrio_4:
    c->rcv_qid.qix = redu_cQixPrio4;
    break;
  case redu_ePrio_5:
    c->rcv_qid.qix = redu_cQixPrio5;
    break;
  case redu_ePrio_6:
    c->rcv_qid.qix = redu_cQixPrio6;
    break;
  case redu_ePrio_7:
    c->rcv_qid.qix = redu_cQixPrio7;
    break;
  case redu_ePrio_8:
    c->rcv_qid.qix = redu_cQixPrio8;
    break;
  case redu_ePrio_9:
    c->rcv_qid.qix = redu_cQixPrio9;
    break;
  case redu_ePrio_10:
    c->rcv_qid.qix = redu_cQixPrio10;
    break;
  }
  c->rcv_qid.nid  = 0;
  qattr.type = qcom_eQtype_private;
  qattr.quota = 100;

  /* Delete the queue if it exists */
  qcom_DeleteQ(&sts, &c->rcv_qid);
	
  /* Create the receiver queue */  
  if (!qcom_CreateQ(&sts, &c->rcv_qid, &qattr, "RedcomRcv"))
    return sts;

  c->send_qid.qix = redu_cQixExport;
  c->send_qid.nid = 0;

  return REDU__SUCCESS;
}

pwr_tStatus redu_receive( redu_tCtx ctx, unsigned int timeout, int *size, void **msg)
{
  pwr_tStatus sts;
  qcom_sGet get;

  memset( &get, 0, sizeof(get));
  get.maxSize = 111000;

  qcom_Get(&sts, &ctx->rcv_qid, &get, timeout);
  if ( sts == QCOM__TMO)
    return sts;

  if ( ODD(sts)) {
    *size = get.size;
    *msg = get.data;
  }
  return sts;
}

pwr_tStatus redu_get_initial_state( char *nodename, int busid, int *state)
{
  pwr_tFileName fname;
  FILE 		*fp;
  char		buffer[256];
  char		name[80];
  char		s_nid[80];
  char		s_naddr[80];
  char		s_port[80];
  char		s_state[10];
  int 		n;
  int 		local_found = 0;
  char		*s;
  
  
  sprintf( fname, pwr_cNameRedcom, "$pwrp_load/", nodename, busid);
  dcli_translate_filename( fname, fname);
  fp = fopen( fname, "r");
  if ( !fp)
    return REDU__REDCOMFILE;

  while ((s = fgets(buffer, sizeof(buffer) - 1, fp)) != NULL) {

    if (*s == '#' || *s == '!') {
      s++;
      continue;
    }

    n = sscanf(s, "%s %s %s %s %s", name, s_nid, s_naddr, s_port, s_state);
    if (n != 5)
      break;

    if ( strcmp( name, nodename) == 0) {
      local_found = 1;
      // *state = atoi(s_state);
      *state = pwr_eRedundancyState_Init;
      break;
    }
  }
  fclose( fp);

  if ( !local_found)
    return REDU__LOCALNODE;
  return REDU__SUCCESS;
}

pwr_tStatus redu_send_table_request( redu_tCtx ctx)
{
  redu_sHeader *msg;
  pwr_tStatus sts;

  timelog( 2, "Redu seding table request");

  msg = (redu_sHeader *)malloc( sizeof(redu_sHeader));
  msg->type = redu_eMsgType_TableRequest;

  sts = redu_send( ctx, msg, sizeof(redu_sHeader), 0);

  free( msg);

  return sts;
}

pwr_tStatus redu_send_table_version( redu_tCtx ctx)
{
  redu_sMsgTableVersion *msg;
  pwr_tStatus sts;

  timelog( 2, "Redu seding table version");

  msg = (redu_sMsgTableVersion *)malloc( sizeof(redu_sMsgTableVersion));
  
  msg->h.type = redu_eMsgType_TableVersion;
  msg->version = ctx->table_version;  

  sts = redu_send( ctx, msg, sizeof(redu_sMsgTableVersion), 0);

  free( msg);

  return sts;
}

pwr_tStatus redu_send_table_version_request( redu_tCtx ctx)
{
  redu_sHeader *msg;
  pwr_tStatus sts;

  timelog( 1, "");

  msg = (redu_sHeader *)malloc( sizeof(redu_sHeader));  
  msg->type = redu_eMsgType_TableVersionRequest;

  sts = redu_send( ctx, msg, sizeof(redu_sHeader), 0);

  free( msg);

  return sts;
}

pwr_tStatus redu_set_state( pwr_eRedundancyState state)
{
  pwr_tStatus sts;
  pwr_tOid oid;
  pwr_sClass_RedcomConfig *p;

  sts = gdh_GetClassList( pwr_cClass_RedcomConfig, &oid);
  if ( EVEN(sts)) return sts;

  sts = gdh_ObjidToPointer( oid, (void **)&p);
  if ( EVEN(sts)) return sts;

  switch ( state) {
  case pwr_eRedundancyState_Active:
    p->SetActive = 1;
    break;
  case pwr_eRedundancyState_Passive:
    p->SetPassive = 1;
    break;
  case pwr_eRedundancyState_Off:
    p->SetOff = 1;
    break;
  default: ;
  }
  return REDU__SUCCESS;
}

pwr_tStatus redu_appl_init( redu_tCtx *ctx, pwr_sClass_RedcomPacket *packetp)
{
  pwr_sNode *nodep;
  pwr_tOid noid;
  pwr_tStatus sts;

  sts = gdh_GetNodeObject( pwr_cNNid, &noid);
  if ( EVEN(sts)) return sts;

  sts = gdh_ObjidToPointer( noid, (void **)&nodep);
  if ( EVEN(sts)) return sts;

  return redu_init( ctx, nodep, packetp);
}

pwr_tStatus redu_appl_send( redu_tCtx ctx, void *msg, int size, pwr_tTime version, unsigned int msg_id)
{
  pwr_tStatus sts;

  ((redu_sMsgHeader *)msg)->h.type = redu_eMsgType_Cyclic;
  ((redu_sMsgHeader *)msg)->size = size;  
  ((redu_sMsgHeader *)msg)->version = version;

  sts = redu_send( ctx, msg, size, msg_id);
  if ( ODD(sts) && ctx->packetp) {
    ctx->packetp->TransmitCnt++;
    ctx->packetp->PacketSize = size;
  }
  return sts;
}

pwr_tStatus redu_appl_receive( redu_tCtx ctx, unsigned int timeout, void **msg, int *size)
{
  pwr_tStatus sts;

  sts = redu_receive( ctx, timeout, size, msg);
  if ( EVEN(sts) && sts != QCOM__TMO) {
    // Wait to avoid looping
    struct timespec  ts = {timeout/1000, (timeout * 1000000) % 1000000000};
    nanosleep(&ts, NULL);
  }
  if ( ODD(sts) && ctx->packetp) {
    ctx->packetp->ReceiveCnt++;
    ctx->packetp->PacketSize = *size;
  }
  return sts;
}