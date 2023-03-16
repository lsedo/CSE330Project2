#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>
#include <linux/stdio.h>

// typedef unsigned 64 bits for timekeeping
typedef unsigned long u64;

//Create 4 required module parameters
static int buffSize = 5, prod = 1, cons = 1, uuid;
module_param(buffSize,int,0644);
module_param(prod,int,0644);
module_param(cons,int,0644);
module_param(uuid,int,0644);

//Create required semaphores
struct semaphore mutex;
struct semaphore full;
struct semaphore empty;

// task_struct pointers
struct task_struct **buffer = NULL;
struct task_struct **consumer_list = NULL;
struct task_struct *producer_thread = NULL;

// Timer and counter to measure the consumer time
static u64 total_time = 0;
static int consumer_count = 0;

// Producer thread
int producer_func(void *args) {
  return 0;
}

// Consumer thread
int consumer_func(void *args) {
  return 0;
}

int producer_consumer_init(void){
  //Initialize semaphores
  sema_init(&mutex,0);
  sema_init(&full,0);
  sema_init(&empty,buffSize);
  
  return 0;
}

void producer_consumer_exit(void){
  
  // Variables for runtime calculation
  u64 seconds = 0, minutes = 0, hours = 0;
  int count 0;
  
  //End Stuff
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");
