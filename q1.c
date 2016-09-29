#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define MAXSTRING 100

int collapse(int number);

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
   int      *numbers;                  // array of numbers from file
   int      counter;                   // just a runner
   int      local_collapse;            // hold a local collapse
   int      global_collapse;           // hold global collapse

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
   f_description = fopen(file_name, "r");
   
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
   numbers = malloc(size * sizeof(int));

   if (numbers == NULL) {
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

         // printf("(%d) low_index: %d\n", current_pid, low_index);
         // printf("(%d) high_index: %d\n", current_pid, high_index);
         // printf("(%d) size: %d\n", current_pid, size);

         for (counter = 0; counter < size; counter++) {
            fread(&number, sizeof(int), 1, f_description);
            // printf("(%d) number: %d\n", p_id, number);
            if (current_pid == 0) numbers[counter] = number;
            else MPI_Send(&number, 1, MPI_INT, current_pid, 0, MPI_COMM_WORLD);
         }
      }
   } else {
      for (counter = 0; counter < size; counter++) {
         MPI_Recv(&numbers[counter], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
   }


   // collapse all the number in local array
   local_collapse = 0;
   for (counter = 0; counter < size; counter++) {
      local_collapse = collapse(local_collapse + numbers[counter]);
      // printf("(%d) number: %d, local_collapse: %d\n", p_id, numbers[counter], local_collapse);
   }
   // printf("(%d) number: %d\n", p_id, local_collapse);

   MPI_Reduce(&local_collapse, &global_collapse, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
   
   elapsed_time += MPI_Wtime();

   if (p_id == 0) {
      printf("INPUT FILE:     %s\n", file_name);
      printf("N:              %d\n", global_size);
      printf("COLLAPSE:       %d\n", collapse(global_collapse));
      printf("PROCESSOR:      %d\n", comm_size);
      printf("TIME:           %.6f seconds\n", elapsed_time);
   }

   // terminate MPI
   MPI_Finalize();

   return 0;
}

int collapse(int number) {
   while (number >= 10) {
      int decimal = number % 10;
      number = (number / 10) + decimal;
   }
   return number;
}