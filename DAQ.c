#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <thread>
#include "dask.h"
#include "conio.h"

#include "TransferFunctions.h"
#include "database.h"

#define MAX_CHAN 4
#define DELAY 1L
I16 card, err;

void my_handler(int s){
  printf("Caught signal %d\nexiting...\n",s);
  Release_Card( card );
  exit(1);
}

void nsleep(long ns) {
  struct timespec tv;
  tv.tv_sec = (time_t) 0;
  tv.tv_nsec = ns;
  nanosleep(&tv, &tv);
}


int main( void ) {
  int testNumber = getTestNumber();
  TransferFunctions func[MAX_CHAN];
  struct timeval tv1, tv2;
  struct timeval sampTime[MAX_CHAN];
  double sensorUpdateThrottle[MAX_CHAN];

  databaseBufferClear();

  for(int i=0; i < MAX_CHAN; i++) {
    std::string transfunc = getSensorTransferFunction(i);
    if(transfunc.compare("")!=0) {
      func[i].setFunction(transfunc);
    } else {
      func[i].setFunction("x");
    }
  }

  for(int i=0; i < MAX_CHAN; i++) {
    sensorUpdateThrottle[i] = (double)getSensorUpdateThrottle(i)/1000.0/1000.0;
    gettimeofday(&sampTime[i], NULL);
  }

  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);


  double totaltime = 0;
  long int count = 0;
  I16 range=AD_U_10_V;
  U16 chan_data[MAX_CHAN];
  F64 chan_voltage[MAX_CHAN];

  setbuf( stdout, NULL );
  if ((card=Register_Card(PCI_9113, 0)) <0 ) {
    printf("Register_Card error=%d\n", card);
    exit(1);
  }
  struct timeval currentTime;
  while(1) {
    gettimeofday(&tv1, NULL);
    for(int i=0 ; i<MAX_CHAN; i++ ){
      gettimeofday(&currentTime, NULL);
      double currentTime_seconds = (double) ((currentTime).tv_usec) / 1000000 + (double) ((currentTime).tv_sec);
      double sampTime_seconds = (double) ((sampTime[i]).tv_usec) / 1000000 +(double) ((sampTime[i]).tv_sec);
      if(currentTime_seconds - sampTime_seconds > sensorUpdateThrottle[i]) {
        if( (err = AI_ReadChannel(card, i, range, &chan_data[i]) ) != NoError )
          printf(" AI_ReadChannel Ch#%d error : error_code: %d \n", i, err );
        gettimeofday(&sampTime[i], NULL);
        AI_VoltScale(card, range, chan_data[i], &chan_voltage[i]);
        bufferSensorData(testNumber,&sampTime[i],i,chan_data[i],func[i].callFunction(chan_voltage[i]));
      } else {

      }
    }

    /*clrscr();
    printf("\n\n\n\n");
    printf("                Ch0        Ch1        Ch2        Ch3        Ch4\n");
    printf(" input value :  %04x       %04x       %04x       %04x       %04x\n", chan_data[0], chan_data[1], chan_data[2], chan_data[3], chan_data[4]);
    printf("     voltage :  %.2f       %.2f       %.2f       %.2f       %.2f\n", chan_voltage[0], chan_voltage[1], chan_voltage[2], chan_voltage[3], chan_voltage[4]);
    printf("     scaledv :  %.2f       %.2f       %.2f       %.2f       %.2f\n\n", func[0].callFunction(chan_voltage[0]), func[1].callFunction(chan_voltage[1]), func[2].callFunction(chan_voltage[2]), func[3].callFunction(chan_voltage[3]), chan_voltage[4]);
    printf("\n\n\n\n\n  press ^C to stop \n");*/

    if(count%10000 == 99) {
      //executeDatabaseWrite();
      std::thread databaseWriterThread(executeDatabaseWrite);
      databaseWriterThread.detach();
    }
    gettimeofday(&tv2, NULL);
    totaltime += ((double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +(double) (tv2.tv_sec - tv1.tv_sec))*1000*1000;
    count++;
    if(count%100000 == 0)
      printf("cylce time: %10.0f us\ncount: %ld\n",totaltime/count, count);
    struct timespec tv;
    nsleep(2000);
  };

  return 0;
}
