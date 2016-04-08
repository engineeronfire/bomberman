/*
 * Userspace program that communicates with the led_vga device driver
 * primarily through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

/* Hanyi Du, hd2342
   Wantong Li, wl2520 */

#include <stdio.h>
#include "vga_led.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int vga_led_fd;

/* Read and print the segment values */
void print_segment_info() {
  vga_led_arg_t vla;
  int i;

  for (i = 0 ; i < VGA_LED_DIGITS ; i++) {
    vla.digit = i;
    if (ioctl(vga_led_fd, VGA_LED_READ_DIGIT, &vla)) {
      perror("ioctl(VGA_LED_READ_DIGIT) failed");
      return;
    }
    printf("%02x ", vla.segments);
  }
  printf("\n");
}

/* Write the contents of the array to the display */
void write_segments(const unsigned int segs[2])
{
  vga_led_arg_t vla;
  int i;
  // alternate between sending x and y coordinates (send x when i = 0, send y when i = 1)
  for (i = 0 ; i < 2; i++) {
      vla.digit = i;
      vla.segments = segs[i];
    if (ioctl(vga_led_fd, VGA_LED_WRITE_DIGIT, &vla)) {
      perror("ioctl(VGA_LED_WRITE_DIGIT) failed");
      continue;
      return;
    }
  }
}

int main()
{
  vga_led_arg_t vla;
  int i=1;
  static const char filename[] = "/dev/vga_led";
  static unsigned int message[2] = {0x10, 0x10}; // initial position

  printf("VGA LED Userspace program started\n");

  if ( (vga_led_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  printf("initial state: ");
  print_segment_info();

  write_segments(message);

  printf("current state: ");
  print_segment_info();

  int xdir = 0;
  int ydir = 0;  
  
  // boundary counditions and bounce policy. The boundaries may be display-specific.
  while(1){
        if(message[0] >= 623)
	    xdir = 1;
	if(message[0] <= 16)
            xdir = 0;
	 if(message[1] >= 463)
	    ydir = 1;
	if(message[1] <= 16)
            ydir = 0;

	if(xdir==0)
	    message[0]=message[0]+i;
	else
	    message[0]=message[0]-i;
	if(ydir==0)
	    message[1]=message[1]+i;
	else
	    message[1]=message[1]-i;
	write_segments(message);
  	usleep(10000);
  
  printf("VGA LED Userspace program terminating\n");
  return 0;
}
