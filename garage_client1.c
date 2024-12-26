#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define BUFSIZE   256  // the size of buffers
#define BLOCKSIZE 64   // the default size of buffers (can not exceed BUFSIZE)

// this will be the console that will handshake with esp32 and samd51 board
// for garage knocker
// usage is to enter the ip address of the server running on the esp32.
// the IP address can be found by opening the web page for the linksys router
// the IP for the linksys router is 192.168.1.1 . If it has changed use ipconf
// after joining the linksys wifi network. password is admin


struct samstat{
  uint8_t proxstate1; // proximity sensor 1
  uint8_t proxstate2; // proximity sensor 2
  int16_t current_user;
  int16_t switch_state; // the state of the 5 switches in their respective bit positions
  int16_t flash_record_size;
  int64_t globaltime; // the current global time
  int32_t flash_raw_offset;
  int32_t number_of_knocks;
  uint32_t deviceinfo;
  uint32_t image_checksum;
};
__attribute__((__aligned__(4))) struct samstat samstatus;


struct pars{  // these are the parameters that are used to check the knock to see if it is useracceptable
  int16_t num_knocks;
  int16_t use_toggle;  // allow for toggle to also be used if !=0
  float min_tspace[8]; // the relative min ratio spacing between successive knocks
  float max_tspace[8]; // the relative max ratio spacing between successive knocks
  int32_t max_absolute_tspace;
  int32_t min_amp; // must have an amplitude at least this big
  float max_amp_ratio;
  int16_t min_base;
  int16_t max_base;
  float min_riseoverfall;
  float max_riseoverfall;
  int32_t max_base_diflimit; // the maximum that the established base and the sample base can differ
  float min_fft1;
  float max_fft1;
  float min_fft2;
  float max_fft2;
  uint32_t image_checksum;
};
__attribute__((__aligned__(4))) struct pars user;


struct knock{
	uint64_t starttime; // the beginning of the capture
        int32_t recordnumber; // this record number
	int32_t recordsize; // this record size
	int32_t raw_offset; // the address ofset for the raw data sections
	uint32_t door_status;  // the door status open/closed etc
	int16_t amp;
	int16_t amp_end; // amp at end of capture used to determine a door movement.
	int16_t base;
	int16_t kpeak; // to keep 32bit aligned
	int32_t basecheck;
	float riseoverfall; // rise slope divided by fall slope
	int16_t kupper; // 75% of peak point
	int16_t klower; // 25% of peak point
	int32_t seq_return;
	float fft1;  // first fft measurement
	float fft2;  // second fft measurement
	int32_t special_record; // special record used for non knock flag
}; // currently 72 bytes

__attribute__((__aligned__(4)))struct knock thisknock; // this is the knock structure




void testlink(void);
void dumpflash(void);
void samd51status(void);
void eraseflash(void);
void resetsamd51(void);
void write_user_param(void);
void read_user_param(void);
void install_user(void);
void printuser(void);
//int32_t limitcheck(int32_t num, int32_t xmin, int32_t xmax);
int32_t limitread(char ztype, char * output,char *  cmin,char * cmax);
void testdownload(void);
void forcedooropenclose(void);
void downloadimage(void);
int32_t sendpreviousret(void);
void dumponerecord(void);
void eraseandrestart(void);
void calcfilechecksum(void);
void printrejectcodes(void);

int32_t lastuser=0;
uint8_t receive_buf[BUFSIZE],send_buf[BUFSIZE];
int sockfd=0;
int32_t current_user=0;

int main(int argc, char *argv[])
{
  setvbuf(stdout, NULL,_IOLBF, 0); // set buffering mode for stdout: flush at \n.
  int32_t k;
  int n = 0;
  char recvBuff[1024];
  struct sockaddr_in serv_addr; 

  if(argc != 2)
    {
      printf("\n Usage: %s <ip of server> \n",argv[0]);
      printf("   Must be on same wifi as esp32 board (default linksys)\n");
      printf("   Current IP address for garage board is 24.4.245.22 if from outside \n");
      printf("   Current IP address for garage board is 10.0.0.134 if laptop on 1265wilson_wifi wifi \n");
      
      printf("   Current IP address for developer board is 10.0.0.99 if on 1265wilson_wifi\n");
      printf(" ---------- This below is obsolete as we are no longer on linksys netowk -------------\n");
      printf(" To find ip address of esp32 server board ... \n");
      printf("    Open browser and type 192.168.1.1 \n");
      printf("    Should be linksys web page . password is  admin \n");
      printf("    Can find the list of connected devices and IP addresses \n");
      printf(" ---------- End of obsolete secotion  ---------------------\n");	
      return 1;
    } 

  memset(recvBuff, '0',sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    } 

  memset(&serv_addr, '0', sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(80); 

  if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
      printf("\n inet_pton error occured\n");
      return 1;
    } 
  printf("connecting...\n");
  fflush(stdout);
  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("\n Error : Connect Failed \n");
      return 1;
    } 
  char sendbuf[20]="trying";
  printf(" \n Success : Socket connected \n");
  printf(" Connected to server running on esp32 board on samd51 board\n");


  //-----------------------------------------
  int cmdnum;
  while(1){
    fflush(stdin);
    //while ((getchar()) != '\n');       // clear the input buffer

    printf(" Enter the number for the command \n\n");
    printf("  0)  Exit \n"
	   "  1)  Send me the number of current status structure including proximity data \n"
	   "  2)  Send me the knocks from n to m \n"  
	   "  3)  Dump flash to file from address x to address y \n"
	   "  4)  Write data to address x in flash \n"
	   "  5)  Erase a number of 128kB blocks of flash starting from address xx \n"
	   "  6)  Read the current xmega structure with record size etc \n"
	   "  7)  Send me the status of the system. (including of garage door is currently open or closed)\n"
	   "  8)  Upload new image to samd51 to execute.\n"
	   "  9)  Upload new image for esp32 to execute.\n" 
	   " 10) Set new user parameters for samd51 in nvm (user1 or user2)\n"
	   " 11) Read current set of user parameters. \n"
	   " 12) Erase all knocks and issue software reset\n"
	   " 13) Test upload link \n"
	   " 14) Control reset GP pins on the esp32. \n"
	   " 15) Write n blocks of data to flash starting from address xx \n"
	   " 16) Dump flash record of one block\n"
	   " 17) Issue software reset to samd51 processor \n"
	   " 18) Force Garage Door to open/close\n"
	   " 19) Force usage of a particular user parameters (1 or 2)\n"
	   " 20) test download link \n"
           " 21) Download new image and reset \n"
           " 22) Get error code from SAMD51 for last command it processed \n"
	   " 23) Calc checksum for a binary file \n" 
	   " 24) printf the bit designation for rejection code \n"
	   "\n");
     fflush(stdout); 

    scanf("%d",&cmdnum); // get the command
    printf(" processing command %d \n",cmdnum);
    if((cmdnum<1)||(cmdnum>32)){
      cmdnum=0;
      printf(" command number out of range \n");
    }

    int32_t j;
    int32_t nblks,sizeofblocks;
    switch( cmdnum){
    case 0: // exit cleanly
      close(sockfd);
      exit(1);
      break;
    case 1: // get status of samd61
      samd51status();
      break;
    case 3: // dump flash to file
      dumpflash();
      break;
    case 5: // erase flash from host
      eraseflash();
      break;
    case 10: // write_user_param
      write_user_param();
      break;
    case 11: // read_user_param
      read_user_param();
      break;
    case 12: // erase all flash and restart
      eraseandrestart();
      break;
    case 13: // test upload link
      testlink();
      break;
    case 16: // dump one flash record and decode (uses command 3 from a sam32 perspective)
      dumponerecord();
      break;
    case 17: // force samd51 software self reset
      resetsamd51();
      break;
    case 18: // force garage door open/close
      forcedooropenclose();
      break;
    case 19: // force samd51 to use a particular user mode
      install_user();
      break;	
    case 20: // test link download
      testdownload();
      break;
    case 21: // download image
      downloadimage();
      break;
    case 22: // previous command error return code
      sendpreviousret();
      break;

    case 23: // calculates the 32 bit checksum for a binary file
      calcfilechecksum();
      break;
    case 24: // printf rejection codes
      printrejectcodes();
      break;
    default:
      printf(" not supported \n");
      break;
    }

  }
  //     n=write(sockfd,sendbuf,sizeof(sendbuf));
  while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
      recvBuff[n] = 0;
      if(fputs(recvBuff, stdout) == EOF)
	{
	  printf("\n Error : Fputs error\n");
	}
    } 

  if(n < 0)
    {
      printf("\n Read error \n");
    } 
  close(sockfd);
  return 0;
}


//--------------------------------------------------------------------
void testlink(void){
  int32_t k,n,nblks,sizeofblocks,j;
  uint8_t nullbuf[BUFSIZE];
  printf(" testing speed of upload link \n\n");
  printf(" Enter the number of blocks \n");
  scanf("%d",&nblks);
  if((nblks<0)||(nblks>2000000))nblks=0;
  printf(" Enter the size of blocks \n");
  scanf("%d",&sizeofblocks);
  if(sizeofblocks<0)nblks=0;
  if(sizeofblocks>BUFSIZE){
    sizeofblocks=BUFSIZE;
    printf(" size too big : defaulting to using %d \n",BUFSIZE);
  }
  
  // if(sizeofblocks !=BLOCKSIZE){
  //  printf(" only current support blocksize same as BLOCKSIZE %d\n",BLOCKSIZE);
  //  nblks=0;
  //}
  
  for(k=0;k<BLOCKSIZE;k++)send_buf[k]=0;
  for(k=0;k<BUFSIZE;k++)nullbuf[k]=0;
  send_buf[0]=13;
  int32_t * xadd;
  xadd=(int32_t *)&send_buf[4];
  *xadd=nblks; // send the number of blocks
  xadd=(int32_t *)&send_buf[8];
  *xadd=sizeofblocks; // send the number of blocks
  
  
  n=write(sockfd,send_buf,BLOCKSIZE); // send command
  printf(" sent the command block \n");
  
  printf(" waiting for %d blocks of size %d \n",nblks,sizeofblocks);
  for(k=0;k<nblks;k++){
    int ngot=0;
    //    memcpy(receive_buf,nullbuf,sizeof(receive_buf));
    while(ngot<sizeofblocks){
      ngot+=read(sockfd,&receive_buf[ngot],sizeofblocks-ngot);
    }
    if(sizeofblocks!=ngot){
      printf(" \n error: block number %d received %d bytes \n",k,ngot);
    }
    
    //	  printf(" received block %d from esp-samd51 system\n",k);
  }
  printf(" completed receiving all %d expected blocks\n",nblks);
  return;
}
//---------------------------------------

void printrejectcodes(void){
  printf(" error reject codes for bits \n \n"
	 " // 0 success \n"
	 " // bit 0 =0x01 one of the knocks is too weak \n"
	 " // bit 1 =0x02 baseline move to much from established baseline\n"
	 " // bit 2 =0x04 amp_ratio out of range\n"
	 " // bit 3 =0x08 baseline out of range\n"
	 " // bit 4=0x10 max spacing too big\n"
	 " // bit 5=0x20 relative spacing not good\n"
	 " // bit 6=0x40 rise over fall out of range\n"
	 " // bit 7=0x80 seems like a noise from garage door moving\n"
	 " // bit 8=0x100 command was between knocks ...\n"
	 " // bit 9=0x200 fft1 (kmean) was out of range\n"
	 " // bit 10=0x400 fft2 (ksigsq) was out of range\n"
	 "\n");
  return;
}
//--------------------------------------------------------------------
void testdownload(void){
  int32_t k,n,nblks,sizeofblocks,j;
  uint8_t nullbuf[BUFSIZE];
  printf(" testing speed of download link \n\n");
  printf(" Enter the number of blocks \n");
  scanf("%d",&nblks);
  if((nblks<0)||(nblks>2000000))nblks=0;
  printf(" Enter the size of blocks \n");
  scanf("%d",&sizeofblocks);
  if(sizeofblocks<0)nblks=0;
  if(sizeofblocks>BUFSIZE){
    sizeofblocks=BUFSIZE;
    printf(" size too big : defaulting to using %d \n",BUFSIZE);
  }
  
  // if(sizeofblocks !=BLOCKSIZE){
  //  printf(" only current support blocksize same as BLOCKSIZE %d\n",BLOCKSIZE);
  //  nblks=0;
  //}
  
  for(k=0;k<BLOCKSIZE;k++)send_buf[k]=0;
  for(k=0;k<BUFSIZE;k++)nullbuf[k]=0;
  send_buf[0]=20;
  int32_t * xadd;
  xadd=(int32_t *)&send_buf[4];
  *xadd=nblks; // send the number of blocks
  xadd=(int32_t *)&send_buf[8];
  *xadd=sizeofblocks; // send the number of blocks
  
  
  n=write(sockfd,send_buf,BLOCKSIZE); // send command
  printf(" sent the command block for command %d\n",(uint8_t)send_buf[0]);
  
  printf(" send %d blocks of size %d \n",nblks,sizeofblocks);
  for(k=0;k<sizeofblocks;k++)receive_buf[k]=k;
  for(k=0;k<nblks;k++){
    int ngot=0;
    while(ngot<sizeofblocks){
      ngot+=write(sockfd,&receive_buf[ngot],sizeofblocks-ngot);
    }
    if(sizeofblocks!=ngot){
      printf(" \n error: block number %d sent %d bytes \n",k,ngot);
    }
    //    usleep(100);
  }
  printf(" completed sending all %d  blocks\n",nblks);
  return;
}


//--------------------------------------------------------------------
void calcfilechecksum(void){
  int32_t k,n,nn,nblks,j;
  uint32_t checksum,checksumsend;
  char fileck[80];
  FILE * ck;
  int32_t err=1;
  printf(" calculate the checksum of a binary file \n\n");
  
  err=1;
  while(err==1){
    err=0;
    printf(" enter the name of the binary file for checksum  \n");
    scanf(" %s",fileck);
    ck=fopen(fileck,"rb");
    if(ck==NULL){
      err=1;
      printf(" error open file %s \n",fileck);
      printf(" try again \\n");
    }
  }



  printf(" now reading through file to get checksum \n");
  // calculate the checksum to check at the end (1 extra block)
  n=0;
  j=1;
  checksum=0;
  nblks=0;
  while(j!=0){ // read till end of file
    for(k=0;k<16;k++)*(uint32_t *)(&send_buf[k<<2])=0;
    //    printf(" cleared 16 4 byte section\n");
    j=fread(send_buf,1,64,ck); // send buffer is used
    //    printf(" return from fread is %d \n",j);
    n+=j;
    for(k=0;k<16;k++){
      checksum+=*(uint32_t*)(&send_buf[k<<2]);
      //     if((n==64)&&(k<3))printf(" first elements 0x%x 0x%x 0x%x 0x%x \n",send_buf[k<<2],send_buf[1+(k<<2)],send_buf[2+(k<<2)],send_buf[3+ (k<<2)]);
    }
    if(j!=0)nblks++;
  }   

  printf(" checksum is 0x%8.8x  \n",checksum);  
}


//--------------------------------------------------------------------
void downloadimage(void){
  int32_t k,n,nn,nblks,j;
  uint32_t checksum,checksumsend;
  char fileimage[80];
  FILE * image;
  int32_t err=1;
  printf(" download new image  \n\n");
  
  err=1;
  while(err==1){
    err=0;
    printf(" enter the file name of the image for the data \n");
    printf(" copy the file into this running directory first is easiest \n");
    scanf(" %s",fileimage);
    image=fopen(fileimage,"rb");
    //    printf(" image pointer is %d \n",(int32_t)image);
    if(image==NULL){
      err=1;
      printf(" error open file %s \n",fileimage);
      printf(" try again \\n");
    }
  }



  printf(" now reading through file to get checksum \n");
  // calculate the checksum to check at the end (1 extra block)
  n=0;
  j=1;
  checksum=0;
  nblks=0;
  while(j!=0){ // read till end of file
    for(k=0;k<16;k++)*(uint32_t *)(&send_buf[k<<2])=0;
    //    printf(" cleared 16 4 byte section\n");
    j=fread(send_buf,1,64,image); // send buffer is used
    //    printf(" return from fread is %d \n",j);
    n+=j;
    for(k=0;k<16;k++){
      checksum+=*(uint32_t*)(&send_buf[k<<2]);
      //     if((n==64)&&(k<3))printf(" first elements 0x%x 0x%x 0x%x 0x%x \n",send_buf[k<<2],send_buf[1+(k<<2)],send_buf[2+(k<<2)],send_buf[3+ (k<<2)]);
    }
    if(j!=0)nblks++;
  }   

  printf(" checksum is 0x%x nblks is %d \n",checksum,nblks);  


  if((nblks<10)||(nblks>128*1024/BLOCKSIZE)){ // allow up to 128kB images
    printf(" image block size (in Bytes) not in expected range... expected %d to %d \n",10*BLOCKSIZE, 128*1024);
  }

  printf(" Number of %d byte blocks to send in image is %d\n",BLOCKSIZE,nblks);

  
  

  // now get the size of the flash in the sam device as it is needed to know where to image
  //  samd51status(); // get the status and fill structure


  for(k=0;k<BLOCKSIZE;k++)send_buf[k]=0;
  send_buf[0]=21;
  int32_t * xadd;
  xadd=(int32_t *)&send_buf[4];
  *xadd=nblks; // send the number of blocks
  xadd=(int32_t *)&send_buf[8];
  *xadd=BLOCKSIZE; // send the number of blocks
  xadd=(int32_t *)&send_buf[12];
  *xadd=checksum; // send the checksum
  nn=0;
  printf(" sending command for image with first 4 32bit values 0x%x 0x%x 0x%x 0x%x \n",*(uint32_t *)&send_buf[0],*(uint32_t *)&send_buf[4],*(uint32_t *)&send_buf[8],*(uint32_t *)&send_buf[12]);
  while(nn<BLOCKSIZE){
    nn+=write(sockfd,send_buf,BLOCKSIZE); // send command
  }
  
  rewind(image);
  n=0;
  j=1;
  checksumsend=0;
  for(n=0;n<nblks;n++){ // read till end of file
    for(k=0;k<16;k++)*(uint32_t*)(&send_buf[k<<2])=0;
    j=fread(send_buf,1,64,image); // send buffer is used
    nn=0;
    if(j!=64){
      printf(" Error: read a 64 byte block but file only returned %d for block %d ... continuing.. \n",j,n);
    }
    //    usleep(500);
    while(nn<BLOCKSIZE){      
      nn+=write(sockfd,&send_buf[nn],BLOCKSIZE-nn); // send command
    }
    for(k=0;k<16;k++){
      checksumsend+=*(uint32_t*)(&send_buf[k<<2]);
    }
  }
  printf(" checksum on sending data is 0x%x \n",checksumsend);
  if(checksum!=checksumsend){
    printf(" Error: checksum does not agree of sending data with the earlier read of file\n");
  }
  printf(" completed sending all %d blocks for dowload of image\n",nblks);
  return;
}

  // will return the previous error return code that the samd51 had
  int32_t sendpreviousret(void){
    int32_t k,n,nblks,sizeofblocks,j;

    for(k=0;k<BLOCKSIZE;k++)send_buf[k]=0;
    send_buf[0]=22; // command 1 is read status of samd51
    int32_t * xadd;
    xadd=(int32_t *)&send_buf[4];
    *xadd=BLOCKSIZE; // send the size of blocks
  
    n=write(sockfd,send_buf,BLOCKSIZE); // send command
    printf(" sent the command block \n");
  
    int ngot=0;
    while(ngot<BLOCKSIZE){
      ngot+=read(sockfd,&receive_buf[ngot],BLOCKSIZE-ngot);
    }
    printf(" SAMD51 error code from last command is %d \n",*(int32_t *)&receive_buf[0]); 
    return(*(int32_t *)&receive_buf[0]);
  }

  //---------------------------------------------------------------
  void samd51status(void){
    // will fill the structure samstatus
    int32_t k,n,nblks,sizeofblocks,j;


    for(k=0;k<BLOCKSIZE;k++)send_buf[k]=0;
    send_buf[0]=1; // command 1 is read status of samd51
    int32_t * xadd;
    xadd=(int32_t *)&send_buf[4];
    *xadd=BLOCKSIZE; // send the size of blocks
  
    n=write(sockfd,send_buf,BLOCKSIZE); // send command
    printf(" sent the command block \n");


    // now read back a single block with the data.
    /*

      struct samstat{
      uint8_t proxstate1; // proximity sensor 1
      uint8_t proxstate2; // proximity sensor 2
      int16_t current_user;
      int16_t switch_state; // the state of the 5 switches in their respective bit positions
      int16_t flash_record_size;
      int64_t globaltime; // the current global time
      int32_t flash_raw_offset;
      int32_t number_of_knocks;

      };
    

    */

  
    int ngot=0;
    while(ngot<BLOCKSIZE){
      ngot+=read(sockfd,&receive_buf[ngot],BLOCKSIZE-ngot);
    }
    memcpy(&samstatus,receive_buf,sizeof(samstatus));
    printf("\n\n Samd51 status : \n"
	   "  proximity state1 is              %d \n"
	   "  proximity state2 is              %d \n"
	   "  current user mode is             %d \n"
	   "  the current switch state is      %d \n"
	   "  record size is                 0x%x \n"
	   "  offset for raw data in record  0x%x \n"
	   "  the global time is             0x%x \n"
	   "  number of records (knocks)is     %d \n"
	   "  the device info                0x%x \n"
	   "  the image checksum used        0x%8.8x \n",
	   samstatus.proxstate1,samstatus.proxstate2,samstatus.current_user,
	   samstatus.switch_state,samstatus.flash_record_size,samstatus.flash_raw_offset,samstatus.globaltime,
	   samstatus.number_of_knocks,samstatus.deviceinfo,samstatus.image_checksum);
    current_user=samstatus.current_user;
    return;
  }
    
    
  //--------------------------------------------
  // will dump one record to screen.  (uses command 3 to read from external flash)
  // will call status to make sure we have filled the status array. Then use this to
  // read the record and parse it as we will know if the offset is the same as the record size
  void dumponerecord(void){
    int32_t k,nbytes,sadd,blocksize,err=1;
    int32_t recordtodump;
    int32_t n,ngot;
    while(err==1){
      err=0;
      printf(" enter the record number to dump (integer) \n");
      scanf(" %d",&recordtodump);
      if((recordtodump<0)||(recordtodump>=(1024*1024*128/8/1024))){
	printf(" error: out of range must be at least 0 and less than 16k \n");
	err=1;
      }
    }


    for(k=0;k<BLOCKSIZE;k++)send_buf[k]=0;
    send_buf[0]=1; // command 1 is read status of samd51
    int32_t * xadd;
    xadd=(int32_t *)&send_buf[4];
    *xadd=BLOCKSIZE; // send the size of blocks
  
    n=write(sockfd,send_buf,BLOCKSIZE); // send command
    printf(" sent the command block to get status for record size and offset \n");
    ngot=0;
    while(ngot<BLOCKSIZE){
      ngot+=read(sockfd,&receive_buf[ngot],BLOCKSIZE-ngot);
    }
    memcpy(&samstatus,receive_buf,sizeof(samstatus));

    printf(" the record size is 0x%x and the offset is 0x%x \n",samstatus.flash_record_size,
	   samstatus.flash_raw_offset);
  
  

    sadd=recordtodump*samstatus.flash_record_size; // allows for either record type
    nbytes=2*64; // get the structure
    blocksize=64; // use this as the default
    send_buf[0]=3;
    *((int32_t *)&send_buf[4])=nbytes;
    *((int32_t *)&send_buf[8])=sadd;
    *((int32_t *)&send_buf[12])=blocksize;
  
    n=write(sockfd,send_buf,blocksize); // send command
    printf(" sent the command block to read 1 formatted 128B record \n");
  
  
    int nblks=nbytes/blocksize;
    int32_t * thisadd;
    for(k=0;k<nblks;k++){
      int ngot=0;
      while(ngot<blocksize){
	ngot+=read(sockfd,&receive_buf[ngot+k*blocksize],blocksize-ngot);
      }
      if(blocksize!=ngot){
	printf(" \n error: block number %d received %d bytes \n",k,ngot);
	return;
      }
    }
    memcpy(&thisknock,receive_buf,sizeof(thisknock)); // get the pieces of the knock

    // now printf out the structure...
    printf(" The formatted structure piece is \n\n "
	   " starttime    0x%16.16x  \n"
	   " recordnumber %d \n"
	   " recordsize   0x%x \n"
	   " raw_offset   0x%x \n"
	   " door_status  0x%x \n"
	   " amplitude    %d  \n"
	   " amplitude at end of capture %d \n"
	   " baseline     %d  \n"
	   " kpeak        %d  \n"
	   " basecheck    %d  \n" 
	   " riseoverfall %f  \n"
	   " kupper (75%% of peak) %d \n"
	   " klower (25%% of peak) %d \n"
	   " sequence check return 0x%x \n"
	   " fft value 1 %f  \n"
	   " fft value 2 %f  \n"
	   " special record code 0x%x  \n",
	   thisknock.starttime,
	   thisknock.recordnumber,
	   thisknock.recordsize,
	   thisknock.raw_offset,
	   thisknock.door_status,
	   thisknock.amp,
	   thisknock.amp_end,
	   thisknock.base,
	   thisknock.kpeak,
	   thisknock.basecheck,
	   thisknock.riseoverfall,
	   thisknock.kupper,
	   thisknock.klower,
	   thisknock.seq_return,
	   thisknock.fft1,
	   thisknock.fft2,
	   thisknock.special_record);
  
    return;
  }
  /*


  
    for(k=0;k<nblks;k++){
    int ngot=0;
    while(ngot<blocksize){
    ngot+=read(sockfd,&receive_buf[ngot],blocksize-ngot);
    }
    if(blocksize!=ngot){
    printf(" \n error: block number %d received %d bytes \n",k,ngot);
    return;
    }
    }
  


    printf(" finished writing %d blocks of %d bytes to binary file %s\n",nblks,blocksize,filename);
    return;
    }

    return;
    }
  */

  //--------------------------------------------
  // will dump the flash form a beginning address
  // format of command:
  // command_block[0] = 3
  // command_block[4-7]= number of bytes
  // command_block[8-11] start address in flash
  // command_block[12-15] block_size for transfer (typically 64)
  // will now offer ot dump all records...

void dumpflash(void){
  int32_t k,nbytes,sadd,blocksize,err=1,all=0;
  while(err==1){
    err=0;
    printf(" enter the number of bytes (hex)  (-1 indicates all records) \n");
    scanf(" %x",&nbytes);
    if(nbytes==-1){
      samd51status();// get the status to get record count.
      nbytes=samstatus.flash_record_size*samstatus.number_of_knocks;
      sadd=0;
      printf(" dumping 0x%x  (%d) bytes from address 0\n",nbytes,nbytes);
      err=0;
      all=1;
    }else{      
      if((nbytes<64)||(nbytes>(1024*1024*128))){
	printf(" error: out of range must be at least 64 and less than 128MB \n");
	err=1;
      }
      if((nbytes & 0x3f)!=0){
	printf(" error: byte number must be multiple of 64 bytes \n");
	err=1;
      }
    }
    if(all==0){
      err=1;
      while(err==1){
	err=0;
	printf(" enter the start address (hex) \n");
	scanf(" %x",&sadd);
	if((sadd<0)||(sadd>(1024*1024*128))){
	  printf(" error: out of range must be at least 0 and less than 128MB \n");
	  err=1;
	}
	if((sadd & 0x3f)!=0){
	  printf(" error: add must be multiple of 64 bytes \n");
	  err=1;
	}
      }
    }
  }


  
  err=1;
  char filename[80];
  FILE * towrite;
  while(err==1){
    err=0;
    printf(" enter the file name to store the data \n");
    scanf(" %s",filename);
    towrite=fopen(filename,"wb");
    if(towrite==00){
      err=1;
      printf(" error open file %s \n",filename);
    }
  }


  
  blocksize=64; // use this as the default

  send_buf[0]=3;
  *((int32_t *)&send_buf[4])=nbytes;
  *((int32_t *)&send_buf[8])=sadd;
  *((int32_t *)&send_buf[12])=blocksize;
  
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" sent the command block \n");
  
  
  int nblks=nbytes/blocksize;


  for(k=0;k<nblks;k++){
    int ngot=0;
    while(ngot<blocksize){
      ngot+=read(sockfd,&receive_buf[ngot],blocksize-ngot);
    }
    if(blocksize!=ngot){
      printf(" \n error: block number %d received %d bytes \n",k,ngot);
      return;
    }
    if(k%1000==999)printf(" writing the %dth 64 byte block\n",k+1);
    fwrite(receive_buf,1,blocksize,towrite); // write to unformatted file
  }
  printf(" finished writing %d blocks of %d bytes to binary file %s\n",nblks,blocksize,filename);
  return;
}

//--------------------------------------------
// will erase a number of 128KB blocks of flash form a beginning address
// format of command:
// command_block[0] = 5 followered by 13
// command_block[4-7]= number of 128kB blocks to erase
// command_block[8-11] start address for erase

void eraseandrestart(void){
  int32_t k,nblocks,sadd,blocksize,err=1;

   
  blocksize=64; // use this as the default
  nblocks=0x400; // the whole flash will be erased just in case
  sadd=0x0; // start at 0
  send_buf[0]=5;
  *((int32_t *)&send_buf[4])=nblocks;
  *((int32_t *)&send_buf[8])=sadd;
   
  int n=write(sockfd,send_buf,blocksize); // send command

  printf(" sent command to erase whole cache \n");
  
  resetsamd51();

  printf(" finished sending command to issue software reset to SAMD51 \n");
  
  return;
}
//--------------------------------------------
// will erase a number of 128KB blocks of flash form a beginning address
// format of command:
// command_block[0] = 5
// command_block[4-7]= number of 128kB blocks to erase
// command_block[8-11] start address for erase

void eraseflash(void){
  int32_t k,nblocks,sadd,blocksize,err=1;

  err=1;
  while(err==1){
    err=0;
    printf(" enter the start address (hex) for erase (must be 128kB boundary\n");
    scanf(" %x",&sadd);
    if((sadd<0)||(sadd>(1024*1024*128))){
      printf(" error: out of range must be at least 0 and less than 128MB \n");
      err=1;
    }
    if((sadd & 0x1ffff)!=0){
      printf(" error: add must be multiple of 128kB bytes \n");
      err=1;
    }
  }

  err=1;
  while(err==1){
    err=0;
    printf(" enter the number of 128kB blocks to erase (hex) (all is 0x400)  \n");
    scanf(" %x",&nblocks);
    if(nblocks==0){
      printf(" nothing to erase ... returning\n");
      return;
    }
    if((nblocks<0)||(nblocks>(1024-sadd/128/1024))){
      printf(" error: out of range or over end of 128MB flash memory (total blocks is 1024) \n");
      err=1;
    }
  }

   
  blocksize=64; // use this as the default

  send_buf[0]=5;
  *((int32_t *)&send_buf[4])=nblocks;
  *((int32_t *)&send_buf[8])=sadd;
   
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" sent the command block \n");
  
  printf(" finished erasing %d blocks starting form address 0x%x\n",nblocks,sadd);
  return;

}
//--------------------------------------------
// will force a the samd51 to do a self reset from software. No data exchange needed
// format of command:
// command_block[0] = 17

void resetsamd51(void){
  int32_t k,nblocks,sadd,blocksize,err=1;
  __attribute__((__aligned__(4)))  char answer[80];
  printf(" This will reset the samd51: Are you sure (Y/N)?\n");
  scanf(" %s",answer);
  if(answer[0]!='Y')return;
  blocksize=64; // use this as the default
  send_buf[0]=17;
   
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" sent the command to reset the samd51 \n");
  
  return;
}

//-------------------------------------------------------
// will force the garage door to open/close (no difference)
// format of command:
// command_block[0] = 18

void forcedooropenclose(void){
  int32_t k,nblocks,sadd,blocksize,err=1;
  blocksize=64; // use this as the default
  send_buf[0]=18;
   
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" sent the command to open/close garage door \n");
  
  return;
}

//--------------------------------------------
// will write the user parameter in the samd chip.
// command will only execute if the user parameters have been read
// This will send the entire user parameter structure
// the user structures stored in internal NVM are of maximum 0x80 bytes (128 bytes) so two 64
// blocks are enough to write the entire structure. We will always transfer 128 bytes for this.
// command_block[0] = 10
// command_block[4-7]= 1 or 2 (the user to be updated)
// command_block[8-11] size of the blocks going to samd51 (always write 128 bytes)

void write_user_param(void){
  int32_t k,nblocks,u,nchange,blocksize,err=1;
  __attribute__((__aligned__(4))) uint8_t send_special[128];

  char cc[80];
  

  printf(" enter the user mode to be modified: must have been first read with command 11 ");
  limitread('i',cc,"1","2");
  sscanf(cc,"%d",&u);

  if(u!=lastuser){
    printf(" Error: last user loaded was %d (0 means nothing loaded), must modify last user loaded\n");
    printf(" Error: must first issue command 11 to read the user parameters \n");
    return;
  }

  printuser();

  nchange=100;
  while(nchange!=0){

    printuser();
    printf("\n choose 0 to exit menu \n");
    printf("\n enter the number to change \n\n");
    limitread('i',cc,"0","16");
    sscanf(cc,"%d",&nchange);
    switch(nchange){
    case 0:
      printf(" exiting sub menu \n");
      break;
    case 1:
      printf(" enter the total number of knocks (integer) (previous %d) \n",user.num_knocks);
      limitread('d',cc,"0","16");
      sscanf(cc,"%d",&user.num_knocks);
      break;
    case 2:
      printf(" enter whether to use toggle switch (0=no) (1=yes) (previous %d)\n",user.use_toggle);
      limitread('d',cc,"0","1");
      sscanf(cc,"%d",&user.use_toggle);
      break;
    case 3:
      printf(" This will set relative spacing for the %d spaces between %d knocks \n",user.num_knocks-1,user.num_knocks);
      printf(" Enter the RELATIVE MINIMUM allowed spacing between each of %d spaces and the time between knocks 1 and 2 \n");
      printf(" The first entry is always 1.0  \n");
      for(k=0;k<(user.num_knocks-2);k++){
	printf(" Enter the RELATIVE MINIMUM allowed spacing between knocks %d and %d (previous %f) \n",k+2,k+3,user.min_tspace[k+1]);
        limitread('f',cc,"0.1","4.0");
	sscanf(cc,"%f",&user.min_tspace[k+1]);
      }
      break;
    case 4:
      printf(" This will set RELATIVE MINIMUM spacing for the %d spaces between %d knocks \n",user.num_knocks-1,user.num_knocks);
      printf(" The first entry is always 1.0  \n");
      for(k=0;(k<user.num_knocks-2);k++){
	printf(" Enter the RELATIVE MAXIMUM spacing (floating point) between knocks %d and %d (previous %f) \n",k+2,k+3,user.max_tspace[k+1]);
        limitread('f',cc,"0.1","10.0");
	sscanf(cc,"%f",&user.max_tspace[k+1]);
      }
      break;
    case 5:
      printf(" Enter (floating point) the absolute space between any two knocks in seconds that is accepted.\n");
      printf(" Previous was %f in seconds \n",(float)user.max_absolute_tspace/100000000);
      limitread('f',cc,"0.1","6.0");
      float ft;
      sscanf(cc,"%f",&ft);
      user.max_absolute_tspace=ft*100000000;
      break;
    case 6:
      printf(" Enter (integer) the minimum amplitude (peak over baseline) of a knock to be accepted (previous %d)\n",user.min_amp);
      limitread('d',cc,"100","2000");
      sscanf(cc,"%d",&user.min_amp);
      break;
    case 7:
      printf(" Enter (float) the maximum ratio allowed for biggest to smallest knock amplitude (previous %f)\n",user.max_amp_ratio);
      limitread('f',cc,"0.1","10.0");
      sscanf(cc,"%f",&user.max_amp_ratio);
      break;
    case 8:
      printf(" Enter (integer) the minimum acceptable baseline (previous %d)\n",user.min_base);
      limitread('d',cc,"10","2000");
      sscanf(cc,"%d",&user.min_base);
      break;
    case 9:
      printf(" Enter (integer) the maximum acceptable baseline (previous %d)\n",user.max_base);
      limitread('d',cc,"100","2000");
      sscanf(cc,"%d",&user.max_base);
      break;
    case 10:
      printf(" Enter (float) the minimum rise over fall for the knock envelope pulse (previous %f)\n",user.min_riseoverfall);
      limitread('f',cc,".01","100.0");
      sscanf(cc,"%f",&user.min_riseoverfall);
      break;
    case 11:
      printf(" Enter (float) the maximum rise over fall for the knock envelope pulse (previous %f)\n",user.max_riseoverfall);
      limitread('f',cc,".01","100.0");
      sscanf(cc,"%f",&user.max_riseoverfall);
      break;
    case 12:
      printf(" Enter (int) the maximum difference between the established baseline and each knocks baseline (previous %d)\n",user.max_base_diflimit);
      limitread('d',cc,"0","500");
      sscanf(cc,"%d",&user.max_base_diflimit);
      break;  
    case 13:
      printf(" Enter (float) the minimum value of fft1 (the kmean) (previous %f)\n",user.min_fft1);
      limitread('f',cc,"0.0","1000.0");
      sscanf(cc,"%f",&user.min_fft1);
      break;
    case 14:
      printf(" Enter (float) the maximum value of fft1 (the kmean) (previous %f)\n",user.max_fft1);
      limitread('f',cc,"1.0","1000000000.0");
      sscanf(cc,"%f",&user.max_fft1);
      break;        
    case 15:
      printf(" Enter (float) the minimum value of fft2 (the ksigsq) (previous %f)\n",user.min_fft2);
      limitread('f',cc,"0.0","1000.0");
      sscanf(cc,"%f",&user.min_fft2);
      break;
    case 16:
      printf(" Enter (float) the maximum value of fft2 (the ksigsq) (previous %f)\n",user.max_fft2);
      limitread('f',cc,"1.0","1000000000.0");
      sscanf(cc,"%f",&user.max_fft2);
      break;        

    default:
      break;
    }
  }
  printf(" These are the current choices that will be set for user mode %d \n",u);
  printuser();
  printf(" Are you sure you want to make these changes (Y/N)? \n");
  char answer[80];
  scanf(" %s",answer);
  if(answer[0]!='Y')return;

  
  blocksize=64; // use this as the default
  
  send_buf[0]=10;
  *((int32_t *)&send_buf[4])=u;
  *((int32_t *)&send_buf[8])=blocksize;


  
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" sent the command block \n");

  memcpy(send_special,&user,sizeof(struct pars));
  // now send out the data
  printf(" first data being sent \n");
  for(k=0;k<16;k++)printf(" 0x%x",send_special[k]);
    
  for(k=0;k<2;k++){
    int ngot=0;
    while(ngot<blocksize){
      ngot+=write(sockfd,&send_special[ngot+k*blocksize],blocksize-ngot);
    }
    if(blocksize!=ngot){
      printf(" \n error: block number %d received %d bytes \n",k,ngot);
      return;
    }
  }
  

  /*


  xadd=(int32_t *)&send_buf[4];
  *xadd=nblks; // send the number of blocks
  xadd=(int32_t *)&send_buf[8];
  *xadd=sizeofblocks; // send the number of blocks
  
  
  n=write(sockfd,send_buf,BLOCKSIZE); // send command
  printf(" sent the command block \n");
  
  printf(" send %d blocks of size %d \n",nblks,sizeofblocks);
  for(k=0;k<sizeofblocks;k++)receive_buf[k]=k;
  for(k=0;k<nblks;k++){
    int ngot=0;
    while(ngot<sizeofblocks){
      ngot+=write(sockfd,&receive_buf[ngot],sizeofblocks-ngot);
    }
    if(sizeofblocks!=ngot){
      printf(" \n error: block number %d sent %d bytes \n",k,ngot);
    }
    
  }
  printf(" completed sending all %d  blocks\n",nblks);
   */


  

  return;
}


//--------------------------------------------
// will read the user parameter in the samd chip.
// command will set a global flag that will be used by the partner write command 
// This will read the entire user param structure
// command_block[0] = 11
// command_block[4-7]= the user mode to read
// command_block[8-11] the blocks size to use for the read (always read 128 bytes)

void read_user_param(void){
  
  __attribute__((__aligned__(4))) uint8_t rec_special[128];

  
  int32_t k,umode,blocksize,err;
  err=1;
  while(err==1){
    err=0;
    printf(" enter the user parameter to request (1 or 2)  \n");
    scanf(" %x",&umode);
    if((umode<1)||(umode>2)){
      printf(" error: out of range: must be be 1 or 2 \n");
      err=1;
    }
  }
  blocksize=64; // use this as the default
  
  send_buf[0]=11;
  *((int32_t *)&send_buf[4])=umode;
  *((int32_t *)&send_buf[8])=blocksize;
  
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" set user mode to %d\n",umode);
  
  // now read back two 64 byte blocks with the data
  for(k=0;k<2;k++){
    int ngot=0;
    while(ngot<blocksize){
      ngot+=read(sockfd,&rec_special[ngot+k*blocksize],blocksize-ngot);
    }
    if(blocksize!=ngot){
      printf(" \n error: block number %d received %d bytes \n",k,ngot);
      return;
    }
  }
  // now copy into structure
  memcpy(&user,rec_special,sizeof(struct pars));
  // now printf out the structure.
  printf("\n\n ------ printing out NVM state for user %d -------\n",umode);
  printuser();
  
   

  
  // send command block

  lastuser=umode;
  
  
  return;
}


//-------------------------------------------------
// This will force the samd51 to use the parameters from one of the users (1 or 2)
// command_block[0] = 19
// command_block[4-7]= the user parameters to force
// no additional data is needed to be written or read

void install_user(void){
  int32_t k,umode,blocksize,err;
  err=1;
  while(err==1){
    err=0;
    printf(" enter the user mode 0=training 1= user1 2=user2 3=user1(for now)  \n");
    scanf(" %x",&umode);
    if((umode<0)||(umode>3)){
      printf(" error: out of range: must be between at least 0 at most 3 \n");
      err=1;
    }
  }

  blocksize=64; // use this as the default

  send_buf[0]=19;
  *((int32_t *)&send_buf[4])=umode;
   
  int n=write(sockfd,send_buf,blocksize); // send command
  printf(" set user mode to %d\n",umode);

  
  return;
}


//--------------------
// will print out the user structure currently loaded.
/*
  struct pars{  // these are the parameters that are used to check the knock to see if it is useracceptable
  int16_t num_knocks;
  int16_t use_toggle;  // allow for toggle to also be used if !=0
  float min_tspace[8]; // the relative min ratio spacing between successive knocks
  float max_tspace[8]; // the relative max ratio spacing between successive knocks
  int32_t max_absolute_tspace;
  int32_t min_amp; // must have an amplitude at least this big
  float max_amp_ratio;
  int16_t min_base;
  int16_t max_base;
  float min_riseoverfall;
  float max_riseoverfall;
  int32_t max_base_diflimit; // the maximum that the established base and the sample base can differ
  float min_fft1;
  float max_fft1;
  float min_fft2;
  float max_fft2;
  };

*/


void printuser(void){
  printf(" 1) int16_t num_knocks %d \n"
	 " 2) int16_t use_toggle %d \n"
	 " 3) float   min_tspace[8] %f %f %f %f %f %f %f %f \n"
	 " 4) float   max_tspace[8] %f %f %f %f %f %f %f %f \n"
	 " 5) int32_t max_absolute_tspace %d (10nsec counts)\n"
	 " 6) int32_t min_amp %d \n"
	 " 7) float   max_amp_ratio %f\n"
	 " 8) int16_t min_base %d\n"
	 " 9) int16_t max_base %d\n" 
	 "10) float min_riseoverfall %f\n"
	 "11) float max_riseoverfall %f\n" 
	 "12) int32_t max_base_diflimit %d\n"
	 "13) float min_fft1 %f\n"
	 "14) float max_fft1 %f\n"
	 "15) float min_fft2 %f\n"
	 "16) float max_fft2 %f\n"
	 "   image checksum  0x%8.8x \n",
	 user.num_knocks,
	 user.use_toggle, 
	 user.min_tspace[0],user.min_tspace[1],user.min_tspace[2],user.min_tspace[3],user.min_tspace[4],user.min_tspace[5],user.min_tspace[6],user.min_tspace[7],
	 user.max_tspace[0],user.max_tspace[1],user.max_tspace[2],user.max_tspace[3],user.max_tspace[4],user.max_tspace[5],user.max_tspace[6],user.max_tspace[7],
	 user.max_absolute_tspace,
	 user.min_amp,
	 user.max_amp_ratio,
	 user.min_base,
	 user.max_base,
	 user.min_riseoverfall,
	 user.max_riseoverfall,
	 user.max_base_diflimit,
	 user.min_fft1,
	 user.max_fft1,
	 user.min_fft2,
	 user.max_fft2,
	 user.image_checksum);


  
  return;
}

int32_t limitread(char ztype, char * output,char *  cmin,char * cmax){
  int err=1;
  int32_t intmin,intmax,hexmin,hexmax,intnum,hexnum;
  float fmin,fmax,fnum;
   
  while(err==1){
    err=0;
    switch(ztype){
    case 'i':
      sscanf(cmin,"%d",&intmin);
      sscanf(cmax,"%d",&intmax);
      printf(" enter decimal between %d and %d \n",intmin,intmax); 
      scanf(" %s",output);
      sscanf(output," %d",&intnum);
      if((intnum<intmin)||(intnum>intmax))err=1;
       
      break;
    case 'd':
      sscanf(cmin,"%d",&intmin);
      sscanf(cmax,"%d",&intmax);
      printf(" enter decimal between %d and %d \n",intmin,intmax); 
      scanf(" %s",output);
      sscanf(output," %d",&intnum);
      if((intnum<intmin)||(intnum>intmax))err=1;
       
      break;
    case 'x':
      sscanf(cmin,"%x",&hexmin);
      sscanf(cmax,"%x",&hexmax);
      printf(" enter hex between 0x%x and 0%x \n",hexmin,hexmax); 
      scanf(" %s",output);
      sscanf(output," %x",&hexnum);
      if((hexnum<hexmin)||(hexnum>hexmax))err=1;
      break;
    case 'f':
      sscanf(cmin,"%f",&fmin);
      sscanf(cmax,"%f",&fmax);
      printf(" enter float between %f and %f \n",fmin,fmax); 
      scanf(" %s",output);
      sscanf(output," %f",&fnum);
      if((fnum<fmin)||(fnum>fmax))err=1;
      break;
    default:
      break;
    }
  }
  return(0);
}
