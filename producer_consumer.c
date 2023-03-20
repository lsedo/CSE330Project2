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
struct task_struct **producer_list = NULL;

// Timer and counter to measure the consumer time
static ul_64 total_runtime = 0;
static int consumer_count = 0;

// Count the number of items produced
static int producer_count = 0;

// Producer thread - finite loop
static int producer_func(void *args) {
  struct task_struct *task;
  int empty_ix = 0;
  
  for_each_process(task) {
    
    if(task->cred->uid.val == uuid) {
      
      // Wait on empty (Acquire semaphore lock)
      if (down_interruptible(&empty)) {
              break;     
      }
      
      // Wait on mutex (Acquire semaphore lock)
      if (down_interruptible(&mutex)) {
              break;
      }
      
      // Produce Item- add to buffer
      empty_ix = 0;
      
      while(empty_ix < buffSize){
            if(buffer[empty_ix] == NULL){
                  buffer[empty_ix] = task;
                  printk("[%s] Produced Item#-%d at buffer index:%d for PID:%d\n",current->comm,producer_count,empty_ix,current->pid);
                  ++producer_count;
                  break;
            }
            ++empty_ix;
      }
      
      // Signal mutex (Release semaphore lock)
      up(&mutex); 
      
      // Signal full (Release semaphore lock)
      up(&full);
      
    }
    
  }
  
  return 0;
  
}


// Consumer thread - infinite loop
static int consumer_func(void *args) {
  
  // Variables for the thread
  struct task_struct *c = NULL;
  int buffer_ix = 0;
  ul_64 elapsed_time = 0, seconds = 0, minutes = 0, hours = 0;
  
  
  while(!kthread_should_stop()) {
    
    // Wait on full (Acquire semaphore lock)
    if (down_interruptible(&full)) {
            break;
    }
    
    // Wait on mutex (Acquire semaphore lock)
    if (down_interruptible(&mutex)) {
            break;
    }
    
    buffer_ix = 0;
    
    // Consume Item- remove from buffer
    while (buffer_ix < buffSize) {
            
            if (buffer[buffer_ix]) {
            
                    c = buffer[buffer_ix];
                    buffer[buffer_ix] = NULL;
                    ++consumer_count;
                    // Calculate elapsed time of process
                    elapsed_time = c->start_time - ktime_get_ns();
                    total_runtime += elapsed_time;
                    seconds = elapsed_time / 1000000000;
                    minutes = seconds / 60;
                    seconds = seconds % 60;
                    hours = minutes / 60;
                    minutes = minutes % 60;
              // print out total time in given format
                    printk("[%s] Consumed Item#-%d on buffer index:%d PID:%d Elapsed Time-%d:%d:%d\n",current->comm,consumer_count,buffer_ix,current->pid,(int)hours,(int)minutes,(int)seconds);
                    break;
             }
             ++buffer_ix;
    }            
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
  
  // Initialize semaphores
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
  if (prod > 0) {
          i = 0;
          producer_list = (struct task_struct**)kmalloc(sizeof(struct task_struct*) * prod, GFP_KERNEL);
    
          while(i < prod){
                producer_list[i] = kthread_run(producer_func, NULL, "producer_thread");
                ++i;
          }
    
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
  
  int i = 0;
  
  //  signal the semaphores for each thread to make sure they are not waiting on the semaphore
  // for loop signals the semaphore once for each thread
  
  for (i = 0; i < cons; ++i) {
    
        up(&full);
  }
  
  for (i = 0; i < cons; ++i) {
    
        kthread_stop(consumer_list[i]);
  
  }
  
  for (i = 0; i < prod; ++i) {
    
        up(&empty);
  }
  
  
  for (i = 0; i < prod; ++i) {
    
        kthread_stop(producer_list[i]);
  
  }
  
  // Calculate total runtime
  seconds = 0; minutes = 0; hours = 0;
  seconds = total_runtime / 1000000000;
  minutes = seconds / 60;
  seconds = seconds % 60;
  hours = minutes / 60;
  minutes = minutes % 60;
  
  printk("The total elapsed time of all processes for UID %d is \t%d:%d:%d\n",uuid,hours,minutes,seconds);
  
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");
