#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#define Max_Limit 20
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
int width, channels, height;
typedef struct
{
   int width;
   int height;
   int channels;
} ImageInfo;
int main()
{
   int i;
   char *shared_memory;
   int size;
   char buff[100];
   int width, channels, height;
   while (1)
   {
      int shm_id = shmget(123, sizeof(ImageInfo), 0666 | IPC_CREAT);
      if (shm_id == -1)
      {
         perror("shmget");
         exit(EXIT_FAILURE);
      }
      // Attach to the shared memory
      ImageInfo *shared_info = (ImageInfo *)shmat(shm_id, NULL, 0);
      if (shared_info == (void *)-1)
      {
         perror("shmat");
         exit(EXIT_FAILURE);
      }
      shared_info->width = -1;
      while (shared_info->width == -1)
      {
         printf("\nWaiting for clients..");

         sleep(2);
      }

      printf("\nClient got Connected");
      width = shared_info->width;
      height = shared_info->height;

      channels = shared_info->channels;

      printf("\nprocessing");

      if (shmdt(shared_info) == -1)
      {
         perror("shmdt");
         exit(EXIT_FAILURE);
      }

      int shmid;
      shmid = shmget(345, INT_MAX, 0666);
      if (shmid == -1)
      {
         perror("shmget");
         exit(EXIT_FAILURE);
      }
      shared_memory = shmat(shmid, NULL, 0);
      if (shared_memory == (void *)-1)
      {
         perror("shmat");
         exit(EXIT_FAILURE);
      }
      char *char_shared_memory = (char *)shared_memory;
      char *gray_img = malloc(width * height * channels);
      char *neg_img = malloc(width * height * channels);
      if (gray_img == NULL)
      {
         printf("Unable to allocate memory for the gray image.\n");
         exit(1);
      }

      for (char *p = char_shared_memory, *pg = gray_img; p != char_shared_memory + width * height * channels; p += channels, pg += 1)
      {
         *pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
         if (channels == 4)
         {
            *(pg + 1) = *(p + 3);
         }
      }
      for (char *np = neg_img, *pg = gray_img; pg != gray_img + width * height * channels; pg += 1, np += 1)
      {
         *np = 255 - *pg;
      }
      size_t img_size = width * height * channels;

      for (int i = 0; i < img_size; i++)
      {
         shared_memory[i] = neg_img[i];
      }
      shared_memory[img_size] = '\0';
      printf("\nImage Send");

      if (shmdt(shared_memory) == -1)
      {
         perror("shmdt");
         exit(EXIT_FAILURE);
      }
   }
   return 0;
}
