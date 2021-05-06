/* elevator_null.c
   Null solution for the elevator threads lab.
   Jim Plank
   CS560
   Lab 2
   January, 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "elevator.h"
#include "finesleep.h"

#define talloc(ty, sz) (ty *)malloc((sz) * sizeof(ty))

void *FINESLEEPER;
pthread_cond_t *waiting; //used to signal to elevator that people are waiting for it.
Person *pcheck;
Dllist check;

void initialize_simulation(Elevator_Simulation *es)
{
  es->v = new_dllist();
  if (dll_empty(es->v))
  { //if waitlist empty
    // printf("\t is waitlist empty?0000\n");
  }
  waiting = talloc(pthread_cond_t, 1);
  pthread_cond_init(waiting, NULL);
  Person *pcheck = (Person *)malloc(256);
  pcheck = NULL;
}

void initialize_elevator(Elevator *e)
{
}

void initialize_person(Person *e)
{

  // printf("\t initializedperson name:%s\n", e->fname);
}

void wait_for_elevator(Person *p)
{
  // printf("\n\n\t IN WAIT FOR ELEVATOR FUNCTION \n\n");
  printf("\t %s WAITING FOR ELEVATOR\n",p->fname);
  // printf("\t ADDING PERSON TO WAITLIST \n");
  pthread_mutex_lock(p->lock);
  if(dll_empty(p->es->v))
  {
    printf("\t about to signal waiting \n");
    pthread_cond_signal(waiting); //signal to elevator that waitlist no longer empty
  }
  dll_append(p->es->v, new_jval_v((void *)p)); // add to waitlist
  pthread_mutex_unlock(p->lock);

  Dllist check; //verify who is added to list. could be deleted if only accessing through person or elevator
  check = dll_last((Dllist)p->es->v);//get person at end of list 
  Person *pcheck;
  pcheck = (Person *)jval_v(dll_val(check));
  printf("\tPerson added to list1: %s\n", pcheck->fname); //last person in waitlist

  pthread_mutex_lock(p->lock);
  while (p->e == NULL)
  {
   printf("\t waiting to be assigned elevator \n");
    pthread_cond_wait(p->cond, p->lock);
    printf("\t ASSIGNED AN ELEVATOR: %d\n", p->e->onfloor);
    
    dll_delete_node(check);  //now remove from waitlist
    // if (dll_empty((Dllist)p->es->v)) //if waitlist empty
    // {
    //   printf("\t waitlist wempty\n");
    // }
  }
  pthread_mutex_unlock(p->lock);
}

void wait_to_get_off_elevator(Person *p)
{
  
  printf("\n\n\t IN WAIT TO GET OFF ELEVATOR FUNCTION %s \n\n", p->fname);
  pthread_mutex_lock(p->lock);
  printf("\n\n\t IN WAIT TO GET OFF ELEVATOR FUNCTION %s \n\n", p->fname);

  if(p->e->door_open){ // while elevator door still open after person getting on
    printf("\n\n\t %s telling elevator to close door \n\n",p->fname);
    pthread_cond_signal(p->e->cond); // tell elevator to close door. 
    printf("\n\n\t %s told elevator to close door, now waiting for it to close. \n\n",p->fname);
    //pthread_cond_wait(p->cond,p->lock); //wait for door to close
    // printf("\n\n\t %s door closed, DO YOU NEED THIS WAIT OR CAN YOU COMBINE IT??? \n\n",p->fname);
  }
  pthread_mutex_unlock(p->lock);
      printf("\n\t  waiting to get to my floor: %d elevator is on %d\n", p->to, p->e->onfloor);

  while (p->e->onfloor != p->to) // while elevator not yet on person's lfoor 
  {
    printf("\n\t  waiting to get to my floor: %d elevator is on %d\n", p->to, p->e->onfloor);
    pthread_cond_wait(p->cond, p->lock);
    printf("\n\t  ON MY FLOOR NOW %d\n", p->to);
  }
  printf("\n\t  READY TO GET OFF ELEVATOR\n");
  //pthread_cond_signal(p->e->cond); // tell person 
  pthread_mutex_unlock(p->lock);
  
}

void person_done(Person *p) //person off elevator
{
  printf("person done\n");
}

// int checkForLoad(Elevator *elevator) // confusing return value; 
// {
//   if (dll_empty(elevator->es->v) == 1)
//   { //if waitlist empty
//     return 1;
//   }
//   else // someone in waitlist
//   {
//     return 0; //waitlist not empty
//   }
// }

// int checkForUnload(Elevator *elevator) // 1 = none to unload
// {
//   // printf("\tCHECKING FOR UNLOAD %d\n", dll_empty(elevator->people));
//   if (dll_empty(elevator->people)) 
//   {
//     return 1;
//   }
//   else
//   {
//     return 0; // elevator not empty
//   }
// }

int load(Elevator *elevator, Dllist check) // load this person onto the elevator.
{
  printf("\n\n\t IN LOAD FUNCTION \n");
  pthread_mutex_lock(elevator->lock);
  //Person *pcheck = (Person *)malloc(256);
  if (jval_v(dll_val(check)) != NULL)
  {
    pcheck = (Person *)jval_v(dll_val(check));
  }

  if (elevator->onfloor != pcheck->from)
  {
    pthread_mutex_unlock(elevator->lock);
    move_to_floor(elevator, pcheck->from); // go to the floor of person waiting
    pthread_mutex_lock(elevator->lock);
  }
  else if (!(elevator->door_open)) // if already on floor, just open door
  {
    pthread_mutex_unlock(elevator->lock);
    open_door(elevator);
  }

  pthread_mutex_lock(elevator->lock);
  pcheck->e = elevator;                                              //set the person's elevator field to this elevator

  pthread_cond_signal(pcheck->cond); // singla waiting for elevator
  while (dll_empty(elevator->people)) // waiting for person to enter in order to close door. 
  {
    printf("\nwaiting for person to enter elevator to close door\n\n");
    pthread_cond_wait(elevator->cond, elevator->lock);
     printf("PERSON ENTERED ELEVATOR\n");
  }
  // pthread_mutex_unlock(elevator->lock);
  // pthread_mutex_lock(elevator->lock);
  printf("\n\tPerson getting on elevator: %s\n", pcheck->fname); //last person in waitlist
  

  pthread_mutex_unlock(elevator->lock);
  close_door(elevator);
  pthread_cond_signal(pcheck->cond); //tell person the door is closed after getting in. 
  //while (dll_empty(elevator->people))
  move_to_floor(elevator, pcheck->to);
  open_door(elevator); //open door
  pthread_mutex_lock(elevator->lock);
    pthread_cond_signal(pcheck->cond); //signal waiting to get off elevator
    printf("\n\t  signal to waiting to get off elevator \n");
  pthread_mutex_unlock(elevator->lock); 

   pthread_cond_signal(pcheck->cond); //tell person the elevator is at their floor with door open. 
  // while(elevator->onfloor != pcheck->to){ //wait until youre at the right floor. 
  //   pthread_cond_wait(elevator->cond,elevator->lock);
  // }
   printf("\n\t END OF LOAD FUNCTION \n\n");
  return 0;
}       

int unload(Elevator *elevator, Dllist check) // unload this person off the elevator.
{
  
  pthread_mutex_lock(elevator->lock);
  
  check = dll_first((Dllist)elevator->people); //get first person on waitlist
  if (jval_v(dll_val(check)) != NULL)
  {
    pcheck = (Person *)jval_v(dll_val(check));
  }
  if (elevator->onfloor != pcheck->to)
  {
    printf("\t %s tried to unload on wrong floor: %d instead of %d\n", pcheck->fname, elevator->onfloor, pcheck->to);
    return 1; // go to the floor of person waiting
  }
  pthread_mutex_unlock(elevator->lock);

  printf("\tEND OF UNLOAD :%s",pcheck->fname);
  return 0;
}

void *elevator(void *arg) //change persons floor
{
  Elevator *elevator = (Elevator *)arg;
  while (1)
  {
    pthread_mutex_lock(elevator->lock);
    printf("\t IN ELEVATOR FUNCTION\n\n");
    while (dll_empty(elevator->people) == 0) //if people in elevator == not empty = 0
    {
      printf(" waiting to unload\n\n");
      pthread_cond_wait(elevator->cond, elevator->lock); //use lock already used to lock thiws
      printf("\tdone waiting to unload\n\n");
    }
    pthread_mutex_unlock(elevator->lock);

    pthread_mutex_lock(elevator->lock);
    while (dll_empty(elevator->es->v)&& dll_empty(elevator->people)) // if nobody is waiting = 1 & if nobody on elevator = 1
    {
      printf("\tWAITING FOR PEOPLE10000\n\n");

      pthread_cond_wait(waiting, elevator->lock); //use lock already used to lock thiws
      printf("DONE WAITING FOR WAITLIST \nNOW LOAD() INTO ELEVATOR...\n");
      Dllist check;
      check = dll_first((Dllist)elevator->es->v); //get first person on waitlist
                                                  // Person *pcheck = (Person *)malloc(256);
      if (jval_v(dll_val(check)) != NULL)
      {
        pcheck = (Person *)jval_v(dll_val(check));
      }
      pthread_mutex_unlock(elevator->lock);

      if (load(elevator, check) == 1) // LOAD PERSON INTO ELEVATOR AND MOVE TO FLOOR
      {
        exit(1);
      }
      printf("\tfully loaded\n\n");
    }
    pthread_mutex_lock(elevator->lock);

    Dllist check;
    check = dll_first((Dllist)elevator->people); //get first person on elevator list.
    //Person *pcheck = (Person *)malloc(256);
    if (jval_v(dll_val(check)) != NULL)
    {
      pcheck = (Person *)jval_v(dll_val(check));
    }
    pthread_mutex_unlock(elevator->lock);

    
    if (unload(elevator, check) == 1)
    { // load and close door
      exit(1);
    }
    printf("\tfully unloaded\n\n\n\n");

    //pthread_mutex_lock(elevator->lock);
    // while (!dll_empty(elevator->people) &&dll_empty(elevator->es->v)) // if the elevator list is empty and the waitlist is empty
    // {
    //   printf("waiting after unloading person SINCE NOBODY IS IN ELEVATOR AND THE WAITLIST IS EMPTY\n");
    //   pthread_cond_wait(elevator->cond, elevator->lock); // wait for 
    // }
     if (!dll_empty((Dllist)elevator->es->v))
    { //if waitlist empty
      printf("\t Global waitlist not empty.\n");
    }
    //pthread_mutex_unlock(elevator->lock);
  }
  return NULL;
}
