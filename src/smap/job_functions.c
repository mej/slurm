/*****************************************************************************\
 *  job_functions.c - Functions related to job display mode of smap.
 *****************************************************************************
 *  Copyright (C) 2002-2006 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Danny Auble <da@llnl.gov>
 *
 *  UCRL-CODE-226842.
 *  
 *  This file is part of SLURM, a resource management program.
 *  For details, see <http://www.llnl.gov/linux/slurm/>.
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

#include "src/common/uid.h"
#include "src/common/node_select.h"
#include "src/common/parse_time.h"
#include "src/smap/smap.h"

static int  _get_node_cnt(job_info_t * job);
static int  _max_procs_per_node(void);
static int  _nodes_in_list(char *node_list);
static void _print_header_job(void);
static int  _print_text_job(job_info_t * job_ptr);

extern void get_job()
{
	int error_code = -1, i, j, recs;
	static int printed_jobs = 0;
	static int count = 0;
	static job_info_msg_t *job_info_ptr = NULL, *new_job_ptr = NULL;
	job_info_t job;
	uint16_t show_flags = 0;

	show_flags |= SHOW_ALL;
	if (job_info_ptr) {
		error_code = slurm_load_jobs(job_info_ptr->last_update,
				&new_job_ptr, show_flags);
		if (error_code == SLURM_SUCCESS)
			slurm_free_job_info_msg(job_info_ptr);
		else if (slurm_get_errno() == SLURM_NO_CHANGE_IN_DATA) {
			error_code = SLURM_SUCCESS;
			new_job_ptr = job_info_ptr;
		}
	} else
		error_code = slurm_load_jobs((time_t) NULL, &new_job_ptr, 
					     show_flags);

	if (error_code) {
		if (quiet_flag != 1) {
			if(!params.commandline) {
				mvwprintw(text_win,
					  main_ycord, 1,
					  "slurm_load_job: %s", 
					  slurm_strerror(slurm_get_errno()));
				main_ycord++;
			} else {
				printf("slurm_load_job: %s\n",
				       slurm_strerror(slurm_get_errno()));
			}
		}
	}

	if (!params.no_header)
		_print_header_job();

	if (new_job_ptr)
		recs = new_job_ptr->record_count;
	else
		recs = 0;
	
	if(!params.commandline)
		if((text_line_cnt+printed_jobs) > count) 
			text_line_cnt--;
	printed_jobs = 0;
	count = 0;
	for (i = 0; i < recs; i++) {
		job = new_job_ptr->job_array[i];
		
		if ((job.job_state != JOB_PENDING)
		    &&  (job.job_state != JOB_RUNNING)
		    &&  (job.job_state != JOB_SUSPENDED)
		    &&  ((job.job_state & JOB_COMPLETING) == 0))
			continue;	/* job has completed */

		if (job.node_inx[0] != -1) {
			job.num_nodes = 0;
			j = 0;
			while (job.node_inx[j] >= 0) {
				job.num_nodes +=
				    (job.node_inx[j + 1] + 1) -
				    job.node_inx[j];
				set_grid(job.node_inx[j],
					 job.node_inx[j + 1], count);
				j += 2;
			}
			
			if(!params.commandline) {
				if((count>=text_line_cnt)
				   && (printed_jobs 
				       < (text_win->_maxy-3))) {
					job.num_procs = (int)letters[count%62];
					wattron(text_win,
						COLOR_PAIR(colors[count%6]));
					_print_text_job(&job);
					wattroff(text_win,
						 COLOR_PAIR(colors[count%6]));
					printed_jobs++;
				} 
			} else {
				job.num_procs = (int)letters[count%62];
				_print_text_job(&job);
			}
			count++;			
		}
		if(count==128)
			count=0;
	}
		
	for (i = 0; i < recs; i++) {
		job = new_job_ptr->job_array[i];
		
		if (job.job_state != JOB_PENDING)
			continue;	/* job has completed */

		if(!params.commandline) {
			if((count>=text_line_cnt)
			   && (printed_jobs 
			       < (text_win->_maxy-3))) {
				job.nodes = "waiting...";
				job.num_procs = (int) letters[count%62];
				wattron(text_win,
					COLOR_PAIR(colors[count%6]));
				_print_text_job(&job);
				wattroff(text_win,
					 COLOR_PAIR(colors[count%6]));
				printed_jobs++;
			} 
		} else {
			job.nodes = "waiting...";
			job.num_procs = (int) letters[count%62];
			_print_text_job(&job);
			printed_jobs++;
		}
		count++;			
		
		if(count==128)
			count=0;
	}

	if (params.commandline && params.iterate)
		printf("\n");

	if(!params.commandline)
		main_ycord++;
	
	job_info_ptr = new_job_ptr;
	return;
}

static void _print_header_job(void)
{
	if(!params.commandline) {
		mvwprintw(text_win, main_ycord,
			  main_xcord, "ID");
		main_xcord += 3;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "JOBID");
		main_xcord += 6;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "PARTITION");
		main_xcord += 10;
#ifdef HAVE_BG
		mvwprintw(text_win, main_ycord,
			  main_xcord, "BG_BLOCK");
		main_xcord += 18;
#endif
		mvwprintw(text_win, main_ycord,
			  main_xcord, "USER");
		main_xcord += 9;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "NAME");
		main_xcord += 10;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "ST");
		main_xcord += 8;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "TIME");
		main_xcord += 5;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "NODES");
		main_xcord += 6;
#ifdef HAVE_BG
		mvwprintw(text_win, main_ycord,
			  main_xcord, "BP_LIST");
#else
		mvwprintw(text_win, main_ycord,
			  main_xcord, "NODELIST");
#endif
		main_xcord = 1;
		main_ycord++;
	} else {
		printf("JOBID ");
		printf("PARTITION ");
#ifdef HAVE_BG
		printf("        BG_BLOCK ");
#endif
		printf("    USER ");
		printf("  NAME ");
		printf("ST ");
		printf("      TIME ");
		printf("NODES ");
#ifdef HAVE_BG
		printf("BP_LIST\n");
#else
		printf("NODELIST\n");
#endif
	}
}

static int _print_text_job(job_info_t * job_ptr)
{
	time_t time_diff;
	int printed = 0;
	int tempxcord;
	int prefixlen = 0;
	int i = 0;
	int width = 0;
	char time_buf[20];
	char tmp_cnt[7];
	uint32_t node_cnt = 0;
	char *ionodes = NULL;
	time_t now_time = time(NULL);

#ifdef HAVE_BG
	select_g_get_jobinfo(job_ptr->select_jobinfo, 
			     SELECT_DATA_IONODES, 
			     &ionodes);
	select_g_get_jobinfo(job_ptr->select_jobinfo, 
			     SELECT_DATA_NODE_CNT, 
			     &node_cnt);
	if(!strcasecmp(job_ptr->nodes,"waiting...")) 
		xfree(ionodes);
#else
	node_cnt = job_ptr->num_nodes;
#endif
	if ((node_cnt  == 0) || (node_cnt == NO_VAL))
		node_cnt = _get_node_cnt(job_ptr);
#ifdef HAVE_BG
	convert_num_unit((float)node_cnt, tmp_cnt, UNIT_NONE);
#else
	sprintf(tmp_cnt, "%d", node_cnt);
#endif
	if(!params.commandline) {
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%c", job_ptr->num_procs);
		main_xcord += 3;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%d", job_ptr->job_id);
		main_xcord += 6;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%.10s", job_ptr->partition);
		main_xcord += 10;
#ifdef HAVE_BG
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%.16s", 
			  select_g_sprint_jobinfo(job_ptr->select_jobinfo, 
						  time_buf, 
						  sizeof(time_buf), 
						  SELECT_PRINT_BG_ID));
		main_xcord += 18;
#endif
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%.8s", 
			  uid_to_string((uid_t) job_ptr->user_id));
		main_xcord += 9;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%.9s", job_ptr->name);
		main_xcord += 10;
		mvwprintw(text_win, main_ycord,
			  main_xcord, "%.2s",
			  job_state_string_compact(job_ptr->job_state));
		main_xcord += 2;
		if(!strcasecmp(job_ptr->nodes,"waiting...")) {
			sprintf(time_buf,"0:00:00");
		} else {
			time_diff = now_time - job_ptr->start_time;
			secs2time_str(time_diff, time_buf, sizeof(time_buf));
		}
		width = strlen(time_buf);
		mvwprintw(text_win, main_ycord,
			  main_xcord + (10 - width), "%s",
			  time_buf);
		main_xcord += 11;

		mvwprintw(text_win, 
			  main_ycord,
			  main_xcord, "%5s", tmp_cnt);
		
		main_xcord += 6;

		tempxcord = main_xcord;
		
		i=0;
		while (job_ptr->nodes[i] != '\0') {
			if ((printed = mvwaddch(text_win,
						main_ycord, 
						main_xcord,
						job_ptr->nodes[i])) < 0) {
				xfree(ionodes);
				return printed;
			}
			main_xcord++;
			width = text_win->_maxx 
				- main_xcord;
			if (job_ptr->nodes[i] == '[')
				prefixlen = i + 1;
			else if (job_ptr->nodes[i] == ',' 
				 && (width - 9) <= 0) {
				main_ycord++;
				main_xcord = tempxcord + prefixlen;
			}
			i++;
		}
		if(ionodes) {
			mvwprintw(text_win, 
				  main_ycord,
				  main_xcord, "[%s]", 
				  ionodes);
			main_xcord += strlen(ionodes)+2;
			xfree(ionodes);
		}

		main_xcord = 1;
		main_ycord++;
	} else {
		printf("%5d ", job_ptr->job_id);
		printf("%9.9s ", job_ptr->partition);
#ifdef HAVE_BG
		printf("%16.16s ", 
		       select_g_sprint_jobinfo(job_ptr->select_jobinfo, 
					       time_buf, 
					       sizeof(time_buf), 
					       SELECT_PRINT_BG_ID));
#endif
		printf("%8.8s ", uid_to_string((uid_t) job_ptr->user_id));
		printf("%6.6s ", job_ptr->name);
		printf("%2.2s ",
		       job_state_string_compact(job_ptr->job_state));
		if(!strcasecmp(job_ptr->nodes,"waiting...")) {
			sprintf(time_buf,"0:00:00");
		} else {
			time_diff = now_time - job_ptr->start_time;
			secs2time_str(time_diff, time_buf, sizeof(time_buf));
		}
		
		printf("%10.10s ", time_buf);

		printf("%5s ", tmp_cnt);
		
		printf("%s", job_ptr->nodes);
		if(ionodes) {
			printf("[%s]", ionodes);
			xfree(ionodes);
		}

		printf("\n");
		
	}
	return printed;
}

static int _get_node_cnt(job_info_t * job)
{
	int node_cnt = 0, round;
	bool completing = job->job_state & JOB_COMPLETING;
	uint16_t base_job_state = job->job_state & (~JOB_COMPLETING);
	static int max_procs = 0;

	if (base_job_state == JOB_PENDING || completing) {
		if (max_procs == 0)
			max_procs = _max_procs_per_node();

		node_cnt = _nodes_in_list(job->req_nodes);
		node_cnt = MAX(node_cnt, job->num_nodes);
		round  = job->num_procs + max_procs - 1;
		round /= max_procs;      /* round up */
		node_cnt = MAX(node_cnt, round);
	} else
		node_cnt = _nodes_in_list(job->nodes);
	return node_cnt;
}

static int _nodes_in_list(char *node_list)
{
	hostset_t host_set = hostset_create(node_list);
	int count = hostset_count(host_set);
	hostset_destroy(host_set);
	return count;
}

/* Return the maximum number of processors for any node in the cluster */
static int   _max_procs_per_node(void)
{
	int error_code, max_procs = 1;
	node_info_msg_t *node_info_ptr = NULL;

	error_code = slurm_load_node ((time_t) NULL, &node_info_ptr,
				params.all_flag);
	if (error_code == SLURM_SUCCESS) {
		int i;
		node_info_t *node_ptr = node_info_ptr->node_array;
		for (i=0; i<node_info_ptr->record_count; i++) {
			max_procs = MAX(max_procs, node_ptr[i].cpus);
		}
		slurm_free_node_info_msg (node_info_ptr);
	}

	return max_procs;
}

