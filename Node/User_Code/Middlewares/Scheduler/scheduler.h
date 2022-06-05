#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

#ifndef SCHTICKS_PER_SEC
#define SCHTICKS_PER_SEC 32000
#elif SCHTICKS_PER_SEC < 10000 || SCHTICKS_PER_SEC > 64516
#error Illegal SCHTICKS_PER_SEC - must be in range [10000:64516]. One tick must be 15.5us .. 100us long.
#endif

typedef uint32_t schtime_t;

#if !HAS_schtick_conv
#define us2schticks(us)   ((schtime_t)( ((long long)(us) * SCHTICKS_PER_SEC) / 1000000))
#define ms2schticks(ms)   ((schtime_t)( ((long long)(ms) * SCHTICKS_PER_SEC)    / 1000))
#define sec2schticks(sec) ((schtime_t)( (long long)(sec) * SCHTICKS_PER_SEC))
#define schticks2ms(sch)   ((uint32_t)(((sch)*(long long)1000    ) / SCHTICKS_PER_SEC))
#define schticks2us(os)   ((uint16_t)(((sch)*(long long)1000000 ) / SCHTICKS_PER_SEC))
// Special versions
#define us2schticksCeil(us)  ((schtime_t)( ((long long)(us) * SCHTICKS_PER_SEC + 999999) / 1000000))
#define us2schticksRound(us) ((schtime_t)( ((long long)(us) * SCHTICKS_PER_SEC + 500000) / 1000000))
#define ms2schticksCeil(ms)  ((schtime_t)( ((long long)(ms) * SCHTICKS_PER_SEC + 999) / 1000))
#define ms2schticksRound(ms) ((schtime_t)( ((long long)(ms) * SCHTICKS_PER_SEC + 500) / 1000))
#endif
	
typedef uint32_t schtime_t;
typedef struct schjob_t schjob_t;	
	
struct schjob_t;  // fwd decl.
	
typedef void (*schjobcb_t) (struct schjob_t*);

struct schjob_t 
{
    struct schjob_t* next;
    schtime_t deadline;
    schjobcb_t  func;
};

void sch_init (void);

void sch_runloop (void);

#ifdef __cplusplus
}
#endif
#endif /* SCHEDULER_H */
