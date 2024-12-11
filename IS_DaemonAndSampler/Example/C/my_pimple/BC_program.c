#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

#define COUPLE_GROUP_DIR "./comms/coupleGroup/"
#define LOCK_FILE "./comms/OpenFOAM.lock"
#define PATCH_FACES_FILE COUPLE_GROUP_DIR "patchFaces"
#define U_IN_FILE COUPLE_GROUP_DIR "U.in"

double v_z = 0.37;
double v_z_old = 0.37;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void write_velocity_to_file(int n, double v_z) {
    FILE *file = fopen(U_IN_FILE, "w");
    if (file == NULL) {
        perror("Error opening U.in file\n");
        exit(EXIT_FAILURE);
    }

    double v_x = 0;
    double v_y = 0;
    int refGrad = 0;
    double refValue = 0;
    int valueFrac = 1;

    fprintf(file, "# Patch: CoupleGroup\n");
    for (int i = 0; i < n; i++) {
        fprintf(file, "(%f %f %f) (%d %d %d) (%f %f %f) (%d %d %d) %d\n",
                v_x, v_y, v_z,
                refGrad, refGrad, refGrad,
                refValue, refValue, v_z,
                refGrad, refGrad, refGrad,
                valueFrac);
    }

    fclose(file);
    printf("write velocity to file\n");
}

int read_patch_faces() {
    FILE *file = fopen(PATCH_FACES_FILE, "r");
    if (file == NULL) {
        perror("Error opening patchFaces file\n");
        exit(EXIT_FAILURE);
    }

    int n;
    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);
    fscanf(file, "%d", &n);
    fclose(file);
    printf("Read patchFaces\n");

    return n;
}

void create_lock_file() {
    int fd = open(LOCK_FILE, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        perror("Error creating lock file\n");
        exit(EXIT_FAILURE);
    }
    close(fd);
    printf("Create OpenFOAM.lock\n");
  }

void* monitor_lock_file(void* arg) {
  while (1) {
    if (access(LOCK_FILE, F_OK) != 0) {
      pthread_mutex_lock(&lock);
      int n = read_patch_faces();
      write_velocity_to_file(n, v_z);
      create_lock_file();
      pthread_mutex_unlock(&lock);
    }
    usleep(100000); // 100 milliseconds
  }
  return NULL;
}

  int main() {
      pthread_t monitor_thread;
      pthread_create(&monitor_thread, NULL, monitor_lock_file, NULL);

      while (1) {
        //          pthread_mutex_lock(&lock);
          printf("Enter v_z (previous=%f): ", v_z_old);
          int input_status = scanf("%lf", &v_z);
          if (input_status != 1) {
              v_z = v_z_old;
          } else {
              v_z_old = v_z;
          }
          //  pthread_mutex_unlock(&lock);
          usleep(100000); // 100 milliseconds
      }

      pthread_join(monitor_thread, NULL);
      return 0;
}
