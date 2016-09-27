#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define MAXSTRING 100

int main(int argc, char *argv[]) {

   int      p_id;                      // Processor ID
   int      comm_size;                 // Number of processors
   double   elapsed_time;              // Execution time
   char     file_name[MAXSTRING];      // hold the file name
   FILE     *f_description;            // hold the file description
   int      global_size;               // the total number of integers in the file
   int      low_index;                 // start index of a process
   int      high_index;                // end index of a process
   int      size;                      // number of elements that a process holds
   long     *numbers;

   MPI_Init(&argc, &argv);

   // Start program
   MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &p_id);
   MPI_Barrier(MPI_COMM_WORLD);

   elapsed_time = -MPI_Wtime();

   if (argc != 2) {
      if (p_id == 0) printf("Command line: %s <file_name>\n", argv[0]);
      MPI_Finalize();
      exit(1);
   }

   // get file name
   strcpy(file_name, argv[1]);
   
   // get the first number
   f_description = fopen("test1.dat", "r");
   
   fread(&global_size, sizeof(int), 1, f_description);
   // printf("global size: %d\n", global_size);

   // global_size = 1003;

   // find a way to partition
   int odd = global_size % comm_size;
   int segment = global_size / comm_size;
   low_index = (segment * p_id) + ((p_id < odd) ? p_id : odd);
   high_index = (segment * (p_id + 1)) + ((p_id < odd) ? p_id : odd - 1);
   size = high_index - low_index + 1;

   // printf("(%d) low_index: %d\n", p_id, low_index);
   // printf("(%d) high_index: %d\n", p_id, high_index);
   // printf("(%d) size: %d\n", p_id, size);

   // allocate memory to hold the number
   number = malloc(size * sizeof(long));

   if (number == NULL) {
      if (p_id == 0) printf ("Can't allocate memory\n");
      MPI_Finalize();
      exit (1);
   }
   
   // distribute data to other processors
   if (p_id == 0) {
      int current_pid;
      int number;

      for (current_pid = 0; current_pid < comm_size; current_pid++) {
         int low_index = (segment * current_pid) + ((current_pid < odd) ? current_pid : odd);
         int high_index = (segment * (current_pid + 1)) + ((current_pid < odd) ? current_pid : odd - 1);
         int size = high_index - low_index + 1;

         // fread(&number, sizeof(int), 1, f_description)

         printf("(%d) low_index: %d\n", current_pid, low_index);
         printf("(%d) high_index: %d\n", current_pid, high_index);
         printf("(%d) size: %d\n", current_pid, size);
      }
      
   }

   
   


   elapsed_time += MPI_Wtime();
   if (p_id == 0) {
      printf("Program finish within %10.6f seconds\n", elapsed_time);
   }

   // terminate MPI
   MPI_Finalize();

   return 0;
}