#ifndef _SLURM_PROTOCOL_ERRNO_H
#define _SLURM_PROTOCOL_ERRNO_H

#include <src/common/slurm_return_codes.h>


/* general communication layer return codes */
#define SLURM_SOCKET_ERROR -1
#define SLURM_PROTOCOL_SUCCESS 0
#define SLURM_PROTOCOL_ERROR -1

/* General Message error codes */
#define SLURM_UNEXPECTED_MSG_ERROR 			-1900
#define SLURM_COMMUNICATIONS_CONNECTION_ERROR 		-1901
#define SLURM_COMMUNICATIONS_SEND_ERROR 		-1902
#define SLURM_COMMUNICATIONS_RECEIVE_ERROR 		-1903
#define SLURM_COMMUNICATIONS_SHUTDOWN_ERROR 		-1904
#define SLURM_PROTOCOL_VERSION_ERROR 			-1910
#define SLURM_PROTOCOL_IO_STREAM_VERSION_ERROR		-1911

/* _info.c/ocommuncation layer RESPONSE_SLURM_RC message codes */
#define SLURM_NO_CHANGE_IN_DATA 			-1920

/* job_mgr.c/job_create */
#define ESLURM_INVALID_PARTITION_NAME			-2000
#define ESLURM_DEFAULT_PARTITION_NOT_SET		-2001
#define ESLURM_ACCESS_DENIED				-2002
#define ESLURM_JOB_MISSING_REQUIRED_PARTITION_GROUP	-2003
#define ESLURM_REQUESTED_NODES_NOT_IN_PARTITION		-2004
#define ESLURM_TOO_MANY_REQUESTED_CPUS			-2005
#define ESLURM_TOO_MANY_REQUESTED_NODES			-2006
#define ESLURM_ERROR_ON_DESC_TO_RECORD_COPY		-2007
#define ESLURM_JOB_MISSING_SIZE_SPECIFICATION		-2008
#define ESLURM_JOB_SCRIPT_MISSING			-2009
#define ESLURM_USER_ID_MISSING				-2010
#define ESLURM_JOB_NAME_TOO_LONG			-2011
#define ESLURM_DUPLICATE_JOB_ID				-2012
#define ESLURM_PATHNAME_TOO_LONG			-2013
#define ESLURM_NOT_TOP_PRIORITY				-2014
#define ESLURM_REQUESTED_NODE_CONFIG_UNAVAILABLE	-2015
#define ESLURM_NODES_BUSY				-2016
#define ESLURM_INVALID_JOB_ID				-2017
#define ESLURM_INVALID_NODE_NAME			-2018
#define ESLURM_WRITING_TO_FILE				-2019
#define ESLURM_TRANSITION_STATE_NO_UPDATE		-2020
#define ESLURM_ALREADY_DONE				-2021

/* partition_mgr.c/update_part */
/* already defined above ESLURM_INVALID_NODE_NAME	-2018 */

/* node_mgr.c/update_node */
/* already defined above ESLURM_ACCESS_DENIED		-2002 */
/* already defined above ESLURM_INVALID_NODE_NAME	-2018 */

/* step_mgr.c */
/* already defined above ESLURM_ACCESS_DENIED		-2202 */

#define SLURM_PROTOCOL_SOCKET_IMPL_ZERO_RECV_LENGTH	-8000
#define SLURM_PROTOCOL_SOCKET_IMPL_NEGATIVE_RECV_LENGTH	-8001
#define SLURM_PROTOCOL_SOCKET_IMPL_NOT_ALL_DATA_SENT	-8002
/* look up an errno value */
extern char * slurm_strerror(int errnum);

/* set an errno value */
extern void slurm_seterrno(int errnum);

/* get an errno value */
extern inline int slurm_get_errno();

/* print message: error string for current errno value */
extern void slurm_perror(char *msg);

#endif


