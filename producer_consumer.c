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

// typedef unsigned 64 bits for timekeeping
typedef unsigned long ul_64;

//Create 4 required module parameters
static int buffSize = 5, prod = 1, cons = 1, uuid = 1000;
module_param(buffSize, int, 0644);
module_param(prod, int, 0644);
module_param(cons, int, 0644);
module_param(uuid, int, 0644);

//Create required semaphores
static struct semaphore mutex, full, empty;

// task_struct pointers
struct task_struct **buffer = NULL;
struct task_struct **consumer_list = NULL;
struct task_struct *producer_thread = NULL;

// Timer and counter to measure the consumer time
static ul_64 total_runtime = 0;
static int consumer_count = 0;

// Count the number of items produced
static int producer_count = 0;

// Producer thread
static int producer_func(void *args) {
  struct task_struct *task;
  
  for_each_process(task) {
    
    if(task->cred->uid.val == uuid) {
      
      // Wait on empty (Acquire semaphore lock)
      down_interruptible(&empty);
      
      // Wait on mutex (Acquire semaphore lock)
      down_interruptible(&mutex);
      
      // Produce Item- add to buffer
      buffer[full] = task;
      printk("[%s] Produced Item#-%d at buffer index:%d for PID:%d\n",current->comm,producer_count,full,current->pid);
      ++producer_count;
      
      // Signal mutex (Release semaphore lock)
      up(&mutex); 
      
      // Signal full (Release semaphore lock)
      up(&full);
      
    }
    
  }
  
  return 0;
  
}


// Consumer thread
static int consumer_func(void *args) {
  
  while(!kthread_should_stop()) {
    
    // Wait on full (Acquire semaphore lock)
    down_interruptible(&full);
    
    // Wait on mutex (Acquire semaphore lock)
    down_interruptible(&mutex);
    
    // Consume Item- remove from buffer and calculate time ****
    
    
    // Signal mutex (Release semaphore lock)
    up(&mutex);
    
    // Signal empty (Release semaphore lock)
    up(&empty);
    
    
  }
  
  return 0;
  
}

// The initial module
int producer_consumer_init(void) {

  // Counter variable
  int i = 0;
  
  // Parameter validation
  if ((prod != 0 && prod != 1) || cons < 0 || buffSize < 1) {
    
    return -1;

  }
  
  
  //Initialize semaphores
  sema_init(&empty,buffSize);
  sema_init(&mutex,1);
  sema_init(&full,0);

  // Buffer initialization
  buffer = (struct task_struct**)kmalloc(sizeof(struct task_struct*) * buffSize, GFP_KERNEL);
  
  while (i < buffSize) {
          buffer[i] = NULL;
          ++i;
  }
  
  
  // consumerList initialization
  if (cons > 0) {
    
          i = 0;
          consumer_list = (struct task_struct**)kmalloc(sizeof(struct task_struct*) * cons, GFP_KERNEL);
    
          while (i < cons) {
                consumer_list[i] = NULL;
                ++i;
          }
  }
  
  
  // The producer_thread will be only created if prod = 1
  if (prod == 1) {
    
          producer_thread = kthread_run(producer_func, NULL, "producer_thread");
    
  }
  
  // Create consumer threads
  if (cons > 0) {
          i = 0;
    
          while (i < cons) {
                  consumer_list[i] = kthread_run(consumer_func, NULL, "consumer_thread");
                  ++i;
          }
  }
  
  return 0;
  
}


// The exit module
void producer_consumer_exit(void) {
  
  // Variables for runtime calculation
  ul_64 seconds = 0, minutes = 0, hours = 0;
  int count = 0;
  
  // Calculate total runtime
  /*
  seconds = 0; minutes = 0; hours = 0;
  seconds = total_runtime / 1000000000;
  minutes = seconds / 60;
  seconds = seconds % 60;
  hours = minutes / 60;
  minutes = minutes % 60;
  */
  
  //End Stuff
  
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");
