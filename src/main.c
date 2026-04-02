#include <asm-generic/ioctls.h>
#include <stdlib.h>
#include "../include/image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t resized = 0;

static void handler(int signo, siginfo_t *info, void *context){
  resized = 1;
}

double findBoxAverage(unsigned char* data, int x, int y, int boxWidth, int boxHeight, int stride){
  double pixelCount = 0;
  for (int row = 0; row < boxHeight; row++){ // Iterate Col
    for (int col = 0; col < boxWidth; col++){ // Iterate Row
      int idx = ((col + x) + stride * (row + y)) * 4;
      int rgbBrightness = (data[idx] + data[idx+1] + data[idx+2])/3;
      pixelCount += rgbBrightness;
    }
  }
  return (pixelCount/(boxWidth * boxHeight));
}

void resize_boxFilter(Image *img){
  // Box Filter 


  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  int displayWidth = w.ws_col;
  int displayHeight = w.ws_row;
  int terminalWidth;
  int terminalHeight;


  int proportionalHeight = (img->height * displayWidth) / (2 * img->width);

  
  if (proportionalHeight <= displayHeight){
    terminalWidth = displayWidth;
    terminalHeight = proportionalHeight;
  }
  else {
    terminalHeight = displayHeight;
    terminalWidth = (2 * img->width * displayHeight) / img->height;

  }
    
  int boxWidth = img->width / terminalWidth;
  int boxHeight = img->height / terminalHeight;

  const char* asciiString = " .:-=+*#%@";
  int numAsciiString = strlen(asciiString) - 1;
  unsigned char* resizedImg = malloc((terminalWidth * terminalHeight) * sizeof(unsigned char));
  

  // Remember to check, to possible do it one bit per time, and to use sigaction instead to compare schemes after color is done

  for (int j = 0; j < terminalHeight; j++){
    size_t boxY0 = (j*img->height/terminalHeight);
    size_t boxY1 = ((j+1)*img->height/terminalHeight);
    for (int i = 0; i < terminalWidth; i++){
      size_t boxX0 = (i*img->width/terminalWidth);
      size_t boxX1 = ((i+1)*img->width/terminalWidth);

      double pixelResult = findBoxAverage(img->imageData, boxX0, boxY0, boxX1-boxX0, boxY1-boxY0, img->width);
      resizedImg[j * terminalWidth + i] = (unsigned char)pixelResult;

    }

    for (int k = 0; k <terminalWidth; k++){
      unsigned char pixelVal = resizedImg[j * terminalWidth + k];
      putchar(asciiString[(int)(numAsciiString * ((double)pixelVal/255.0))]);
    }
    putchar('\n');
  }
  free(resizedImg);
}



int main(int argc, char *argv[])
{

  struct sigaction act = {0};
  act.sa_flags = SA_SIGINFO | SA_RESTART;
  act.sa_sigaction = &handler;

  if (sigaction(SIGWINCH, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }


  int width = 0;
  int height = 0;
  int n = 4;

  unsigned char* data = stbi_load(argv[1], &width, &height, &n, 4);

  Image img = {.width = width, .imageData = data, .height = height, .depth = n};
  
  resize_boxFilter(&img);
  while (1){
    pause();
    if (resized){
      resized = 0;
      write(STDOUT_FILENO, "\033[H\033[2J", 7);
      resize_boxFilter(&img);
    }
  }
  

  stbi_image_free(data);




  return 0;
}




