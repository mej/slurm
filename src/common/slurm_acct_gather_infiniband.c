/*****************************************************************************\
 *  slurm_acct_gather_infiniband.c - implementation-independent job infiniband
 *  accounting plugin definitions
 *****************************************************************************
 *  Copyright (C) 2013 Bull.
 *  Written by Yiannis Georgiou <yiannis.georgiou@bull.net>
 *
 *  This file is part of SLURM, a resource management program.
 *  For details, see <http://www.schedmd.com/slurmdocs/>.
 *  Please also read the included file: DISCLAIMER.
 *
 *  SLURM is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of portions of this program with the OpenSSL library under
 *  certain conditions as described in each individual source file, and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify file(s) with this exception, you may extend this
 *  exception to your version of the file(s), but you are not obligated to do
 *  so. If you do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source files in
 *  the program, then also delete it here.
 *
 *  SLURM is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with SLURM; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "src/common/macros.h"
#include "src/common/plugin.h"
#include "src/common/plugrack.h"
#include "src/common/slurm_protocol_api.h"
#include "src/common/xmalloc.h"
#include "src/common/xstring.h"
#include "src/common/slurm_acct_gather_infiniband.h"
#include "src/slurmd/slurmstepd/slurmstepd_job.h"

typedef struct slurm_acct_gather_infiniband_ops {
	int (*update_node_infiniband)	(void);
	void (*conf_options)	(s_p_options_t **full_options,
					   int *full_options_cnt);
	void (*conf_set)	(s_p_hashtbl_t *tbl);
} slurm_acct_gather_infiniband_ops_t;
/*
 * These strings must be kept in the same order as the fields
 * declared for slurm_acct_gather_infiniband_ops_t.
 */
static const char *syms[] = {
	"acct_gather_infiniband_p_update_node",
	"acct_gather_infiniband_p_conf_options",
	"acct_gather_infiniband_p_conf_set",
};

static slurm_acct_gather_infiniband_ops_t ops;
static plugin_context_t *g_context = NULL;
static pthread_mutex_t g_context_lock =	PTHREAD_MUTEX_INITIALIZER;
static bool init_run = false;

extern int slurm_acct_gather_infiniband_init(void)
{
	int retval = SLURM_SUCCESS;
	char *plugin_type = "acct_gather_infiniband";
	char *type = NULL;

	if (init_run && g_context)
		return retval;

	slurm_mutex_lock(&g_context_lock);

	if (g_context)
		goto done;

	type = slurm_get_acct_gather_infiniband_type();

	g_context = plugin_context_create(
		plugin_type, type, (void **)&ops, syms, sizeof(syms));

	if (!g_context) {
		error("cannot create %s context for %s", plugin_type, type);
		retval = SLURM_ERROR;
		goto done;
	}
	init_run = true;

done:
	slurm_mutex_unlock(&g_context_lock);
	xfree(type);
	if (retval == SLURM_SUCCESS)
                retval = acct_gather_conf_init();


	return retval;
}

extern int acct_gather_infiniband_fini(void)
{
	int rc;

	if (!g_context)
		return SLURM_SUCCESS;

	init_run = false;
	rc = plugin_context_destroy(g_context);
	g_context = NULL;

	return rc;
}

extern int acct_gather_infiniband_g_update_node(void)
{
	int retval = SLURM_ERROR;

	if (slurm_acct_gather_infiniband_init() < 0)
		return retval;

	retval = (*(ops.update_node_infiniband))();

	return retval;
}


extern void acct_gather_infiniband_g_conf_options(s_p_options_t **full_options,
                                              int *full_options_cnt)
{
        if (slurm_acct_gather_infiniband_init() < 0)
                return;
        (*(ops.conf_options))(full_options, full_options_cnt);
}

extern void acct_gather_infiniband_g_conf_set(s_p_hashtbl_t *tbl)
{
        if (slurm_acct_gather_infiniband_init() < 0)
                return;

        (*(ops.conf_set))(tbl);
}
