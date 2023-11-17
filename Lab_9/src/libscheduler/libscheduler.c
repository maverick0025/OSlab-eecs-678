/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t
{
  int priority;
  int arrivalTime;
  int remainingTime;
  int runningTime;
  int firstTimeOnCore;
  int lastUpdateTimeOnCore;
  int jobNumber;
  int queueCounter;
} job_t;

priqueue_t* jobQueue;

scheme_t currentScheme;

int numOfCores;
job_t** coreJobs;

int updatedTime;

int waitCount = 0;
float totalWaitTime = 0.0;

float totalResponseTime = 0.0;
int responsesCount = 0;

float totalTurnaroundTime = 0.0;
int turnaroundCount = 0;

int counter = 0;

//FCFS
int comparatorForFCFS(const void * a, const void * b)
{
	job_t* aa = ((job_t*) a);
  job_t* bb = ((job_t*) b);
  return aa->arrivalTime - bb->arrivalTime;
}

//SJF
int comparatorForSJF(const void * a, const void * b)
{
  job_t* aa = ((job_t*) a);
  job_t* bb = ((job_t*) b);
  return aa->runningTime - bb->runningTime;}

// PSJF
int comparatorForPSJF(const void * a, const void * b)
{
	job_t* aa = ((job_t*) a);
  job_t* bb = ((job_t*) b);
  return aa->remainingTime - bb->remainingTime;
}

//PRI and PPRI
int comparatorforPRIAndPPRI(const void * a, const void * b)
{
	job_t* aa = ((job_t*) a);
  job_t* bb = ((job_t*) b);
  return aa->priority - bb->priority;
}

//RR
int comparatorForRR(const void * a, const void * b)
{
	job_t* aa = ((job_t*) a);
  job_t* bb = ((job_t*) b);
  return aa->queueCounter - bb->queueCounter;
}


void updateTime(int time)
{
  updatedTime = time;
  job_t * job = NULL;
 
  for(int i = 0; i < numOfCores; i++) {

    job = coreJobs[i];

    if(job != NULL) {
      if(job->firstTimeOnCore == -1 && job->lastUpdateTimeOnCore != updatedTime) {

        job->firstTimeOnCore = job->lastUpdateTimeOnCore;

        totalResponseTime += job->firstTimeOnCore - job->arrivalTime;
        responsesCount++;

      }

      job->remainingTime -= updatedTime - job->lastUpdateTimeOnCore;
      job->lastUpdateTimeOnCore = updatedTime;

    }
  }
}


/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  jobQueue = (priqueue_t*)malloc(sizeof(priqueue_t));
  switch (scheme){
    case FCFS:
      priqueue_init(jobQueue, comparatorForFCFS);
      break;
    case SJF:
      priqueue_init(jobQueue, comparatorForSJF);
      break;
    case PSJF:
      priqueue_init(jobQueue, comparatorForPSJF);
      break;
    case PRI:
      priqueue_init(jobQueue, comparatorforPRIAndPPRI);
      break;
    case PPRI:
      priqueue_init(jobQueue, comparatorforPRIAndPPRI);
      break;
    case RR:
      priqueue_init(jobQueue, comparatorForRR);
      break;
  }

  numOfCores = cores;
  currentScheme = scheme;

  coreJobs = (job_t**)malloc(sizeof(job_t*)*numOfCores);

  for(int i = 0; i < numOfCores; i++) {
    coreJobs[i] = NULL;
  }

  updateTime(0);
}


/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumption:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
  updateTime(time);
  job_t* job = (job_t*)malloc(sizeof(job_t));
  job->priority = priority;
  job->arrivalTime = time;
  job->remainingTime = running_time;
  job->runningTime = running_time;
  job->firstTimeOnCore = -1;
  job->lastUpdateTimeOnCore = -1;
  job->jobNumber = job_number;
  job->queueCounter = counter;
  counter = counter + 1;

  int firstAvailableCore = -1;

  for(int i = 0; i < numOfCores; i++) {
    if(coreJobs[i] == NULL) {
      firstAvailableCore = i;
      break;
    }
  }

  if(firstAvailableCore != -1) {

    coreJobs[firstAvailableCore] = job;
    job->lastUpdateTimeOnCore = updatedTime;
    return firstAvailableCore;

  } else if(currentScheme == PPRI || currentScheme == PSJF) {

    int result = 0;
    int newResult = 0;
    int index = -1;

    for(int i = 0; i < numOfCores; i++) {

      switch (currentScheme){
        case FCFS:
          newResult = comparatorForFCFS(job, coreJobs[i]);
          break;
        case SJF:
          newResult = comparatorForSJF(job, coreJobs[i]);
          break;
        case PSJF:
          newResult = comparatorForPSJF(job, coreJobs[i]);
          break;
        case PRI:
          newResult = comparatorforPRIAndPPRI(job, coreJobs[i]);
          break;
        case PPRI:
          newResult = comparatorforPRIAndPPRI(job, coreJobs[i]);
          break;
        case RR:
          newResult = comparatorForRR(job, coreJobs[i]);
          break;
      }

      if(newResult < result) {

        result = newResult;
        index = i;

      } else if(newResult == result){

        if(index != -1){
          index = (coreJobs[index]->arrivalTime >= coreJobs[i]->arrivalTime) ? index : i;
        }

      }

    }

    if(index >= 0) {
      job_t* jobTemp = coreJobs[index];
      jobTemp->lastUpdateTimeOnCore = -1;
      coreJobs[index] = NULL;
      difcompare = 1;
      priqueue_offer(jobQueue, jobTemp);
      difcompare = 0;
      coreJobs[index] = job;
      job->lastUpdateTimeOnCore = updatedTime;
      return index;
    } else {
      priqueue_offer(jobQueue, job);
      return -1;
    }

    

  } else {

    priqueue_offer(jobQueue, job);
    return -1;

  }

}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{

  updateTime(time);

  job_t* job = coreJobs[core_id];
  job->lastUpdateTimeOnCore = -1;
  coreJobs[core_id] = NULL;
  //priqueue_offer(jobQueue, job);
  priqueue_remove(jobQueue, job);

  totalWaitTime += updatedTime - job->arrivalTime - job->runningTime;
  waitCount++;

  totalTurnaroundTime += updatedTime - job->arrivalTime;
  
  turnaroundCount++;

  //free(job);

  job = priqueue_poll(jobQueue);

  if(job) {
    coreJobs[core_id] = job;
    job->lastUpdateTimeOnCore = updatedTime;
    return job->jobNumber;
  }

	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{

  updateTime(time);

  job_t* job1 = coreJobs[core_id];
  job1->lastUpdateTimeOnCore = time;
  job1->queueCounter = counter;
  counter ++;
  coreJobs[core_id] = NULL;
  priqueue_offer(jobQueue, job1);

  job_t* job = priqueue_poll(jobQueue);

  if(job) {
    coreJobs[core_id] = job;
    job->lastUpdateTimeOnCore = updatedTime;
    return job->jobNumber;
  }

	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return (float) totalWaitTime / waitCount;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return (float) totalTurnaroundTime / turnaroundCount;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return (float) totalResponseTime / responsesCount;
}


/**
  Free any memory associated with your scheduler.
 
  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  void* temp = NULL;
  while((temp = priqueue_poll(jobQueue)) != NULL) {
    free((job_t*) temp);
  }

  free(coreJobs);
  priqueue_destroy(jobQueue);
  free(jobQueue);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
  //print queue with the following format:
  // jobid(priority) jobid(priority) ...
  for(int i = 0; i < priqueue_size(jobQueue); i++){
    job_t* temp = priqueue_at(jobQueue, i);
    printf("%d(%d) ", temp->jobNumber, temp->priority);
  }
  printf("\n");
}