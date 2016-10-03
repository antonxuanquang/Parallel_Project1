#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define MAXLENGTH 100

int main(int argc, char *argv[]) {

	int 	p_id;						// Processor ID
	int 	comm_size;					// Number of processors
	double 	elapsed_time;				// Execution time
	char 	input_file[MAXLENGTH];		// hold the file name
	FILE 	*in_description;			// hold the file description
	int 	counter;					// just a runner
	int 	low_index;					// start index of a process
	int 	high_index;					// end index of a process
	int 	global_row;					// total rows in an input file
	int 	global_column;				// total columns in an input file
	int 	size;						// number of rows that a process holds
	char 	*line;						// hold a line of character from input file
	size_t 	len = 0;					// length of a line in input file
	ssize_t read;						// if read > 0, we have just read something

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
	strcpy(input_file, argv[1]);

	// open input file
	in_description = fopen(input_file, "r");

	// read first line
	getline(&line, &len, in_description);
	// if (p_id == 0) printf("First line: %s\n", line);
	
		
	// read second line
	getline(&line, &len, in_description);
	// if (p_id == 0) printf("Second line: %s\n", line);
	
	// read third line
	getline(&line, &len, in_description);
	// if (p_id == 0) printf("Third line: %s\n", line);
	sscanf(line, "%d %d", &global_column, &global_row);
	// if (p_id == 0) printf("global_row: %d; global_column: %d\n", global_row, global_column);
	
	// read fourth line
	getline(&line, &len, in_description);
	// if (p_id == 0) printf("Fourth line: %s\n", line);

	// file a way to partition
	int odd = global_row % comm_size;
	int segment = global_row / comm_size;
	low_index = (segment * p_id) + ((p_id < odd) ? p_id : odd);
	high_index = (segment * (p_id + 1)) + ((p_id < odd) ? p_id : odd - 1);
	size = high_index - low_index + 1;

	// printf("(%d) low_index: %d\n", p_id, low_index);
	// printf("(%d) high_index: %d\n", p_id, high_index);
	// printf("(%d) size: %d\n", p_id, size);
	
	if (p_id == 0) {
		int current_pid;

		for (current_pid = 0; current_pid < comm_size; current_pid++) {

			int low_index = (segment * current_pid) + ((current_pid < odd) ? current_pid : odd);
			int high_index = (segment * (current_pid + 1)) + ((current_pid < odd) ? current_pid : odd - 1);
			int size = high_index - low_index + 1;

			// printf("(%d) low_index: %d\n", current_pid, low_index);
			// printf("(%d) high_index: %d\n", current_pid, high_index);
			// printf("(%d) size: %d\n", current_pid, size);

			for (counter = 0; counter < size; counter++) {
				read = getline(&line, &len, in_description);
				// printf("(%d) global_column: %d\n", current_pid, global_column);
				MPI_Send(line, global_column * 2 - 1, MPI_CHAR, current_pid, 0, MPI_COMM_WORLD);	
			}
		}
	}
	for (counter = 0; counter < size; counter++) {
		MPI_Recv(line, global_column * 2 - 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("(%d) length: %zu, line: %s\n", p_id, strlen(line), line);
	}

    fclose(in_description);

	elapsed_time += MPI_Wtime();

	if (p_id == 0) {
		printf("Print output pmg image\n");
		printf("TIME:           %.6f seconds\n", elapsed_time);
	}

	// terminate MPI
	MPI_Finalize();

	return 0;
}