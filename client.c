#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <limits.h>
#define Max_Limit 20
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
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
    char buff[Max_Limit];
    int shmid;
    int width, height, channels;
    char str[Max_Limit];
    //Enter the Name of the Image - Include format name as well
    printf("Enter the Image Name : ");
    scanf("%s", str);
    strcpy(buff, str);

    unsigned char *img = stbi_load(str, &width, &height, &channels, 0);
    if (img == NULL)
    {
        printf("Error in loading the image\n");
        exit(EXIT_FAILURE);
    }

    int shm_id = shmget(123, sizeof(ImageInfo), 0666);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    ImageInfo *shared_info = (ImageInfo *)shmat(shm_id, NULL, 0);
    if (shared_info == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    // Waiting till Server is free
    while (shared_info->width != -1)
    {
        sleep(5);
    }
    // Client Executing when Server is free
    if (shared_info->width == -1)
    {

        shared_info->width = width;

        shared_info->height = height;

        shared_info->channels = channels;

        if (shmdt(shared_info) == -1)
        {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }
        size_t img_size = width * height * channels;
        shmid = shmget(345, INT_MAX, 0666 | IPC_CREAT);
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
        if (img_size > INT_MAX)
        {
            fprintf(stderr, "Image size exceeds the shared memory segment size.\n");
            exit(EXIT_FAILURE);
        }
        // Assigning data to shared memory
        for (int i = 0; i < img_size; i++)
        {
            shared_memory[i] = img[i];
        }
        shared_memory[img_size] = '\0';

        sleep(2);
        char* tok = strtok(buff,".");
        strcat(tok, "_Negative.jpg");
        // Generating Negative Image using shared memory
        stbi_write_jpg(buff, width, height, 1, shared_memory, width);
        printf("\n Image Generated");
        stbi_image_free(img);
        if (shmdt(shared_memory) == -1)
        {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
