#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>

#define SERVER_HOST "192.168.1.1"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128

/*
 * References:
 *
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

int sockfd; /* Socket file descriptor */
pthread_mutex_t receiverow_mutex;
struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

pthread_t network_thread;

//global variable
int receiverow=0;

void *network_thread_f(void *);
char transASCII(char modifier, char keycode); 

void fbclean(int startr, int endr){
  int i,j;
  for(j=startr;j<=endr;j++){
   for(i=0;i<128;i++){
   fbputchar(' ',j,i);

    }
  }
}
int main()
{
  int err, col;
  struct sockaddr_in serv_addr;

  struct usb_keyboard_packet packet;
  int transferred;
  char keystate[88];
  char result;
  int i;
  int rowtmp;
  int coltmp;
  // define the start #row and end #row for send and receive
  int receiveStart,receiveEnd,sendStart,sendEnd;
  receiveStart=0;
  receiveEnd=43;
  sendStart=receiveEnd+2;
  sendEnd=47;
 // keep track of the total number of words being input
  int wordcount=0;

 //keep track of where to display next input character
  int sendrow,sendcol;
  sendrow=0;
  sendcol=0;
  
  char sendPacket[256]; // maximam 2 rows


/*  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }

*/
  //Initialization
//  fbclean(receiveStart,receiveEnd);//clean the receive region

  for (col = 0 ; col < 128 ; col++) { // draw a line to seperate the screen
  //  fbputchar('_', receiveEnd+1, col);
 
  }
 
    //fbclean(sendStart,sendEnd);//clean the send region
  

  /* Open the keyboard */
  if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
    fprintf(stderr, "Did not find a keyboard\n");
    exit(1);
  }
    
  /* Create a TCP communications socket */
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    fprintf(stderr, "Error: Could not create socket\n");
    exit(1);
  }

  /* Get the server address */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  if ( inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Error: Could not convert host IP \"%s\"\n", SERVER_HOST);
    exit(1);
  }

  /* Connect the socket to the server */
 /* if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error: connect() failed.  Is the server running?\n");
    exit(1);
  }
*/
  /* Start the network thread */
  //pthread_create(&network_thread, NULL, network_thread_f, NULL);

  /* Look for and handle keypresses */

  for (;;) {
    
    libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 0);

sprintf(keystate, "%d %d %d %d %d %d %d %d %d %d %d %d", packet.modifiers,packet.reserved, packet.keycode[0],
	      packet.keycode[1],packet.keycode[2],packet.keycode[3],packet.keycode[4],packet.keycode[5],packet.keycode[6],packet.keycode[7],packet.keycode[8],packet.keycode[9]);
 printf("%s\n", keystate);
    fprintf(stderr,"this is a test\n");
 //   fprintf(stderr, "%d\n", packet);
    usleep(200000);
    if (transferred == sizeof(packet)) {
 
     

     sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
	      packet.keycode[1]);
      printf("%s\n", keystate);
     
          

 
  
      if (packet.keycode[0] == 0x29) { /* ESC pressed? */
        printf("esc\n");
	break;
      }
     
      else if(packet.keycode[0] == 0x28){// if enter pressed
      //clean the send region       
           printf("enter\n");
           fbclean(sendStart,sendEnd);

        //send the message to the server through sockit
/*        if(wordcount<=127){

        write(sockfd, sendPacket, wordcount+1);}

        else{
        write(sockfd, sendPacket, 128);
        
        write(sockfd, sendPacket, wordcount+1-128);

        }
*/     if(wordcount>255){wordcount=255;}
      //  write(sockfd, sendPacket, wordcount+1);
   
        //display the sent message on the receive region, and wrap long message acroos multiple line     
        fbputs("me:",receiverow,0);
        col=3;
      
       printf("wordcount: %d",wordcount);
              
        for(i=0;i<wordcount;i++){
                
               if (receiverow>receiveEnd){
                            pthread_mutex_lock(&receiverow_mutex);
                            receiverow=0;
                            pthread_mutex_unlock(&receiverow_mutex);

                            fbclean(receiveStart,receiveEnd);
                       }
               
             
               fbputchar(sendPacket[i],receiverow,col);
               col++;
               if(col==128){
                     col=0;
                     pthread_mutex_lock(&receiverow_mutex);
                     receiverow++;
                     pthread_mutex_unlock(&receiverow_mutex);

                    }
                    

               }
               pthread_mutex_lock(&receiverow_mutex);
               receiverow++;
               pthread_mutex_unlock(&receiverow_mutex);
            
               sendrow=0;
               sendcol=0;
               wordcount=0;
       }









      else if(packet.keycode[0]==0x2a){// if backspace pressed
      
      if(wordcount>0){
   
        
          
         if(sendrow*128+sendcol==wordcount){ // if delete the last character  
          fbputchar(' ',sendrow+sendStart,sendcol);
         }else{

           for(i=sendrow*128+sendcol;i<=wordcount;i++){
                                 
                  sendPacket[i-1]=sendPacket[i];// shift the location of character

             }
         }
        
         wordcount--;
         printf("wordcount:%d\n",wordcount);
           
         //calculate the cursor location            
              if(sendcol>0){
                 sendcol--;       
              }
             
             else{
                 sendrow--;
                 sendcol=127;
             }
          fbputchar('_',sendrow+sendStart,sendcol);

         
        // if the delete one is not the last character
       
       // redisplay the character followed by the cursor                   
           rowtmp=sendrow;
           coltmp=sendcol;

           coltmp++;
           if(coltmp==128){
           rowtmp++;
           coltmp=0;
           }
           for(i=rowtmp*128+coltmp;i<wordcount;i++){

             fbputchar(sendPacket[i],rowtmp+sendStart,coltmp);

             coltmp++;
             if(coltmp==128){
                   coltmp=0;
                   rowtmp++;
             }


          }
          if(sendrow*128+sendcol<wordcount)
          fbputchar(' ',rowtmp+sendStart,coltmp);
   
        
       
      }
}




      else if(packet.keycode[0]==0x4f){// if right arrow  pressed
            

          if(sendrow*128+sendcol<wordcount){// if the cursor is not at the last location,redisplay the character at the cursor location
            if(wordcount<256){
               fbputchar(sendPacket[sendrow*128+sendcol],sendrow+sendStart,sendcol);
    
              //calculate the cursor location                   
                  sendcol++;
                  if(sendcol==128){
                        sendrow++;
                        sendcol=0;
                   }
                                                          
                 fbputchar('_',sendrow+sendStart,sendcol);
                        
               }

          }
      }




      else if(packet.keycode[0]==0x50){// if left arrow  pressed
          

          if(sendcol!=0||sendrow!=0){

               
          
                  if(sendrow*128+sendcol==wordcount){// if it is the last character, the cursor location diplays a blackspace

                       fbputchar(' ', sendrow+sendStart, sendcol);

                    }
                  else{ // if it is the character in the middle, redisplay the character at the cursor location

                  fbputchar(sendPacket[sendrow*128+sendcol],sendrow+sendStart,sendcol);

                  }

                
                     //calculate the cursor location
                    sendcol--;
                    if(sendcol==-1){
                        sendrow--;
                        sendcol=127;
                   }
                
                   fbputchar('_', sendrow+sendStart, sendcol);

                
          }   
      }

     else{ // character is pressed?
       if(sendrow*128+sendcol<256){
            // translate the keycode to ASCII
            result=transASCII(packet.modifiers, packet.keycode[0]);
            
            
           if(result!=0){
            
             // if insert character in the middle of the input

             printf("wordcount: %d",wordcount);

             if(sendrow*128+sendcol<wordcount){
                 
                 //shift right
                 for(i=wordcount+1;i>sendrow*128+sendcol;i--){
                    sendPacket[i]=sendPacket[i-1];                      
                }
              }

              sendPacket[sendrow*128+sendcol]=result;
              fbputchar(result, sendrow+sendStart, sendcol);
             
              wordcount++;             
              if(wordcount>256){wordcount=256;}
                if(wordcount<256){// calculate the location of cursor

                    if(sendcol<127){
                          sendcol++;
                       }
                       
                     else{
                         sendrow++;
                         sendcol=0;
                     }
                if(sendrow!=2){
                         fbputchar('_', sendrow+sendStart,sendcol);   
                   } 
                    
                }
               //redisplay the character followed the cursor
               rowtmp=sendrow;
               coltmp=sendcol;
               if(coltmp<127){
                          coltmp++;
                       }
                     else{
                         rowtmp++;
                         coltmp=0;
                    }
                   for(i=rowtmp*128+coltmp;i<wordcount;i++){
                       if(wordcount==255)break;
                         fbputchar(sendPacket[i],rowtmp+sendStart,coltmp);
                  
                   if(coltmp<127){
                          coltmp++;
                   }
                      
                  else{
                         rowtmp++;
                         coltmp=0;

                  }
                }                       
        printf("input:%s\n",sendPacket);
       
    }
  }
 }
}} 
 /* Terminate the network thread */
  pthread_cancel(network_thread);

  /* Wait for the network thread to finish */
  pthread_join(network_thread, NULL);

  return 0;
}

void *network_thread_f(void *ignored)
{
  char recvBuf[BUFFER_SIZE];
  int n;
 
  int i=0;
  int col=0;
  
  int receiveEnd=43;
  //int receiveStart=0;
  
  /* Receive data */
while(1){
 
   while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
    recvBuf[n] = '\0';
    printf("%s", recvBuf);
    i=0;
    col=0;    

   
     while ( recvBuf[i]!= 0) { 
            
            if(receiverow>receiveEnd){// if receiverow>receive region, clean the screen

         
                     pthread_mutex_lock(&receiverow_mutex);
                     receiverow=0;
                     pthread_mutex_unlock(&receiverow_mutex);

                     fbclean(0,receiveEnd);

          


           }

           fbputchar(recvBuf[i], receiverow, col++);

          
            i++;

            if(i>127){ 


              col=0;          
              pthread_mutex_lock(&receiverow_mutex);
              receiverow++;
              pthread_mutex_unlock(&receiverow_mutex);


          }
    }
     
 
      pthread_mutex_lock(&receiverow_mutex);
      receiverow++;
      pthread_mutex_unlock(&receiverow_mutex);

      col=0;


   }
 }

  return NULL;
}




char transASCII(char modifier, char keycode){

char result=0;

   //spacebar
   if(keycode==0x2C){
      result=' ';
   }
   
   //letter
   else if(keycode>=0x04&&keycode<=0x1D){
        if(modifier==0x02||modifier==0x20)
           result=keycode+61;  //upper case
        else
           result=keycode+93; // lower case
    }

   // 0
   else if(keycode==0x27){
       if (modifier!=0x02 || modifier!=0x20){
             result='0';
     }

   }

   //number 1-9 
   else if(keycode>=0x1e&&keycode<0x27){

       if (modifier!=0x02 || modifier!=0x20){
            result=keycode+19;
             

       }

   }

    else
      result=0x00;

    return result;

}
/*
void cleanSreen(int startrow,int endrow){
int i,j;
for(i=startrow;i<=endrow;i++){
    for(j=0;j<128;j++){

              fbputchar(' ',i,j);


}



}
}
*/


