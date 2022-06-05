#include "scheduler.h"
#include "scheduler_hal.h"
#include<string.h>

// RUNTIME STATE
static struct 
{
	schjob_t* scheduledjobs;
	schjob_t* runnablejobs;
}SCH;

void sch_init() 
{
	memset(&SCH, 0x00, sizeof(SCH));
	hal_disableIRQs();
	
	hal_enableIRQs();
}

schtime_t sch_getTime() 
{
	return hal_ticks();
}

// unlink job from queue, return if removed
static int unlinkjob (schjob_t** pnext, schjob_t* job) 
{
	for( ; *pnext; pnext = &((*pnext)->next)) 
	{
		if(*pnext == job) 
		{ // unlink
			*pnext = job->next;
			return 1;
		}
	}
	return 0;
}

// clear scheduled job
void sch_clearCallback (schjob_t* job) 
{
	hal_disableIRQs();
	unlinkjob(&SCH.scheduledjobs, job) || unlinkjob(&SCH.runnablejobs, job);
	hal_enableIRQs();
}

// schedule immediately runnable job
void sch_setCallback (schjob_t* job, schjobcb_t cb)
{
	schjob_t** pnext;
	hal_disableIRQs();
	// remove if job was already queued
	unlinkjob(&SCH.runnablejobs, job);
	// fill-in job
	job->func = cb;
	job->next = NULL;
	// add to end of run queue
	for(pnext=&SCH.runnablejobs; *pnext; pnext=&((*pnext)->next));
	*pnext = job;
	hal_enableIRQs();
}

// schedule timed job
void sch_setTimedCallback (schjob_t* job, schtime_t time, schjobcb_t cb) 
{
	schjob_t** pnext;
	hal_disableIRQs();
	// remove if job was already queued
	unlinkjob(&SCH.scheduledjobs, job);
	// fill-in job
	job->deadline = time;
	job->func = cb;
	job->next = NULL;
	// insert into schedule
	for(pnext=&SCH.scheduledjobs; *pnext; pnext=&((*pnext)->next)) 
	{
		if((*pnext)->deadline - time > 0) 
		{ 
			// (cmp diff, not abs!)
			// enqueue before next element and stop
			job->next = *pnext;
			break;
		}
	}
	*pnext = job;
	hal_enableIRQs();
}

// execute jobs from timer and from run queue
void sch_runloop() 
{
	while(1) 
	{
		schjob_t* j = NULL;
		hal_disableIRQs();
		// check for runnable jobs
		if(SCH.runnablejobs) 
		{
			j = SCH.runnablejobs;
			SCH.runnablejobs = j->next;
		} 
		else if(SCH.scheduledjobs && hal_checkTimer(SCH.scheduledjobs->deadline)) 
		{ 
			// check for expired timed jobs
			j = SCH.scheduledjobs;
			SCH.scheduledjobs = j->next;
		} 
		else 
		{ 
			// nothing pending
			hal_sleep(); // wake by irq (timer already restarted)
		}
		hal_enableIRQs();
		if(j) 
		{ 
			// run job callback
			j->func(j);
		}
	}
}
