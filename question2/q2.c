#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define MAXLENGTH 100
#define ADDRESS(row, global_column, column) (((row) + (row) * (global_column)) + (column))

void append(char subject[], const char insert[], int pos);
char* deblank(char* input);
void paintFill(char *lines, int row, int column);
int paintFillRecursive(char *lines, int row, int column,
	int global_row, int global_column, int color);

int main(int argc, char *argv[]) {

	int 	p_id;						// Processor ID
	int 	comm_size;					// Number of processors
	double 	elapsed_time;				// Execution time
	char 	input_file[MAXLENGTH];		// hold the file name
	char 	output_file[MAXLENGTH];		// hold the file name
	FILE 	*in_description;			// hold the file description
	FILE 	*out_description;			// hold the file description
	int 	counter;					// just a runner
	int 	low_index;					// start index of a process
	int 	high_index;					// end index of a process
	int 	global_row;					// total rows in an input file
	int 	global_column;				// total columns in an input file
	int 	size;						// number of rows that a process holds
	char 	*line;						// hold a line of character from input file
	char 	*lines;			
	size_t 	len = 0;					// length of a line in input file
	ssize_t read;						// if read > 0, we have just read something

	MPI_Init(&argc, &argv);

	// Start program
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &p_id);
	MPI_Barrier(MPI_COMM_WORLD);

	elapsed_time = -MPI_Wtime();

	if (argc != 2) {
	  if (p_id == 0) printf("Command line: %s <input_image>\n", argv[0]);
	  MPI_Finalize();
	  exit(1);
	}

	// get file name
	strcpy(input_file, argv[1]);
	strcpy(output_file, input_file);
	append(output_file, ".out", strlen(input_file) - 4);

	// open input file
	in_description = fopen(input_file, "r");
	out_description = fopen(output_file, "w");

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
	// lines = (char *) malloc(global_row);
	
	// read fourth line
	getline(&line, &len, in_description);
	// if (p_id == 0) printf("Fourth line: %s\n", line);

	// file a way to partition
	int odd = global_row % comm_size;
	int segment = global_row / comm_size;
	low_index = (segment * p_id) + ((p_id < odd) ? p_id : odd);
	high_index = (segment * (p_id + 1)) + ((p_id < odd) ? p_id : odd - 1);
	size = high_index - low_index + 1;

	// char lines[size][global_column];				// hold all lines to process
	// line = (char *) malloc ((global_column * 2) * sizeof (char));
	lines = (char *) malloc(size * global_column * sizeof(char));

	// printf("(%d) low_index: %d\n", p_id, low_index);
	// printf("(%d) high_index: %d\n", p_id, high_index);
	// printf("(%d) size: %d\n", p_id, size);
	
	// process 0 send all data to all processes, including himself
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
				// printf("(%d) line: %s\n", current_pid, line);
				MPI_Send(line, global_column * 2 - 1, MPI_CHAR, current_pid, 0, MPI_COMM_WORLD);	
			}
		}
	}

	// all processes receive data from process 0
	for (counter = 0; counter < size; counter++) {
		MPI_Recv(line, global_column * 2 - 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// printf("(%d) length: %zu, line: %s\n", p_id, strlen(line), line);
		line = deblank(line);
		// printf("(%d) line: %s\n", p_id, line);
		strcpy(&lines[ADDRESS(counter, global_column, 0)], line);
		// printf("(%d) line: %s\n", p_id, lines[counter]);
	}

	paintFill(lines, global_row, global_column);

	for (counter = 0; counter < size; counter++) {
		// int column;
		// printf("(%d) line: ", p_id);
		// for (column = 0; column < global_column; column++) {
		// 	printf("%c", lines[ADDRESS(counter, global_column, column))]);
		// }
		// printf("\n");
		printf("(%d) line: %s\n", p_id, &lines[ADDRESS(counter, global_column, 0)]);
	}

	   

	elapsed_time += MPI_Wtime();

	if (p_id == 0) {
		printf("Print output pmg image\n");
		printf("TIME:           %.6f seconds\n", elapsed_time);
	}

	fclose(in_description);
	fclose(out_description);

    // free(line);
    // free(lines);

	// terminate MPI
	MPI_Finalize();

	return 0;
}

// inserts into subject[] at position pos
void append(char subject[], const char insert[], int pos) {
    char buf[MAXLENGTH] = {}; 

    strncpy(buf, subject, pos); // copy at most first pos characters
    int len = strlen(buf);
    strcpy(buf+len, insert); // copy all of insert[] at the end
    len += strlen(insert);  // increase the length by length of insert[]
    strcpy(buf+len, subject+pos); // copy the rest

    strcpy(subject, buf);   // copy it back to subject
}

char* deblank(char* input) {
    int i,j;
    char *output = input;
    for (i = 0, j = 0; i<strlen(input); i++,j++) {
        if (!isspace(input[i])) output[j] = input[i];                     
        else j--;                                     
    }
    output[j] = 0;
    return output;
}

void paintFill(char *lines, int row, int column) {
	int i, j, color = 2, old_color = -1;
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			old_color = paintFillRecursive(lines, i, j, row, column, color);
			if (old_color != -1) color++;
		}
	}
}

int paintFillRecursive(char *lines, int row, int column, int global_row, int global_column, int color) {
	
	if (column < 0 || row < 0 || column == global_column || row == global_row) {
		return - 1;	// out-of-bound
	}
	if (lines[ADDRESS(row, global_column, column)] == '1') {
		lines[ADDRESS(row, global_column, column)] = color + '0';
		paintFillRecursive(lines, row + 1, column, global_row, global_column, color);
		paintFillRecursive(lines, row - 1, column, global_row, global_column, color);
		paintFillRecursive(lines, row, column + 1, global_row, global_column, color);
		paintFillRecursive(lines, row, column - 1, global_row, global_column, color);
		return color;
	}
	return -1;
}