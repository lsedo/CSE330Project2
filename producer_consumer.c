#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/stdio.h>

//Create 4 required module parameters
int buffSize, prod, cons, uuid;
module_param(buffSize,int,0644);
module_param(prod,int,0644);
module_param(cons,int,0644);
module_param(uuid,int,0644);

//Create required semaphores
struct semaphore mutex;
struct semaphore full;
struct semaphore empty;

int producer_consumer_init(void){
  //Initialize semaphores
  sema_init(&mutex,0);
  sema_init(&full,0);
  sema_init(&empty,buffSize);
}

void producer_consumer_exit(void){
  //End Stuff
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");
