#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <semaphore.h>
#include <ctype.h>

/*
 //Data structures specified from assignment description
 int *available;  // available amount of each resource
 int *max;   	// demand of each customer
 int *allocated; 	// amount currently allocated to each customer
 int *need;     	//remaining needs of each customer.
 */

/* universal variable declaration*/
typedef struct Customer {
	//Data structures specified from assignment description
	int *available;  // available amount of each resource
	int *max;   	// demand of each customer
	int *allocated; 	// amount currently allocated to each customer
	int *need;     	//remaining needs of each customer.
} Customer;

Customer *resources_ptr;

int *resources_arg;
int customers;
int resources;

/* main functions */
int get_resources(int argc, char *args[]);
void get_status();
void execute_customers();
char* resource_request(int customer_number, int *request);
char* resource_release(int customer_number, int *request);

/* helper functions for main functions*/
bool safe_sequence(int *seq[]);
char* request_handling(char *input, int size, char* (*func)(int, int*));
int* string_to_array(char *delimeter, char *message, int length);
void output_print(int *arr, int len);

/* start */

int main(int argc, char *args[]) {

	/* get resources from run arguments*/
	if (argc < 3) {
		printf("Please enter correct arguments.\n");
		return 1;
	} else {
		get_resources(argc, args);
	}

	/* Keep Program running while user enters inputs or until user enters exit command */
	int is_running = 1;
	char *input = NULL;
	ssize_t console_in = 0; //using this variable to read each line from console
	size_t input_size = 0; // stores address of the variable that holds the size of the input buffer

	/* display general information */

	int temp = resources + 1;
	printf("Number of Customers: %d \n", temp);
	printf("Currently Available Resources: ");
	output_print(resources_arg, resources);

	printf("Maximum resources from file: \n");

	for (int idx = 0; idx < customers; idx++) {
		output_print(resources_ptr[idx].need, resources);
	}

	while (is_running == 1) {
		printf("Enter Command: ");
		console_in = getline(&input, &input_size, stdin);
		if (console_in == -1) { //error handling
			is_running = 0;
		}

		else {
			/* start with case handling
			 * convert input to lower case characters*/
			char *inputchar_pointer = input;
			for (; *inputchar_pointer; ++inputchar_pointer) {
				if (*inputchar_pointer == '\n') {
					*inputchar_pointer = '\0';
				} else
					*inputchar_pointer = tolower(*inputchar_pointer);
			}

			/* RQ: requesting resources*/
			/* make sure request leave system safe*/
			if (input[0] == 'r' && input[1] == 'q' && strlen(input) >= 2) {
				printf("%s",
						request_handling(input, input_size, resource_request));
			}

			/* RL: release resources and updates data structure*/
			else if (strlen(input) >= 2 && input[0] == 'r' && input[1] == 'l') {
				printf("%s",
						request_handling(input, input_size, resource_release));
			}

			/* if user enters Status*/
			else if (strcmp(input, "status") == 0) {
				get_status();
			}

			/* if user enters Run*/
			else if (strcmp(input, "run") == 0) {
				execute_customers();

			}
			/* if user enters Exit*/
			else if (strcmp(input, "exit") == 0) {
				is_running = 0;
				printf("Program Ended\n");
			}
			// otherwise...
			else
				printf("Invalid Command\n");
		}
	}

	return 0;
}

int get_resources(int argc, char *args[]) {

	/* variable init*/
	resources = argc - 1;
	resources_arg = (int*) malloc((resources) * sizeof(int));

	FILE *file_pointer;
	char *line;
	size_t input_size = 0;
	ssize_t curr_line;

	/* Get resources from program arguments*/
	for (int idx = 0; idx < resources; idx++) {
		resources_arg[idx] = atoi(args[idx + 1]);
	}

	/* File Parsing */
	if ((file_pointer = fopen("sample4_in.txt", "r")) == NULL) { // error handling
		printf("File opening error.");
		return -1;
	}

	customers = 1;

	/*
	 * Commented out cause it was giving me problems using char instead
	 while (((char) fgetc(fp)) != EOF) {
	 if (((char) fgetc(fp))) {
	 customers++;

	 if (((char) fgetc(fp)) == EOF) {
	 customers--;
	 }
	 }
	 }
	 */

	char file_char;

	while ((file_char = fgetc(file_pointer)) != EOF) {
		if (file_char == '\n') {
			customers++;
			if ((file_char = fgetc(file_pointer)) == EOF) { //if line is empty
				customers--;
			}
		}
	}

	/* offset file pointer to beginning of file */

	fseek(file_pointer, 0, SEEK_SET);

	resources_ptr = malloc(customers * sizeof(Customer));
	int count = 0;

	/* parse file in customer */
	while ((curr_line = getline(&line, &input_size, file_pointer)) != -1) {
		if (strlen(line) > 1) {

			Customer customer;
			customer.max = string_to_array(",", line, resources);
			customer.allocated = malloc(sizeof(int) * resources);
			customer.need = malloc(sizeof(int) * resources);

			for (int i = 0; i < resources; i++) {
				customer.allocated[i] = 0;
				customer.need[i] = customer.max[i];
			}

			resources_ptr[count] = customer;
			count++;
		}
	}

	fclose(file_pointer);

	return 0;

}

void get_status() {
	printf("Available Resources:\n");
	output_print(resources_arg, resources);

	printf("Maximum Resources:\n");
	for (int idx = 0; idx < customers; idx++) {
		output_print(resources_ptr[idx].max, resources);
	}

	printf("Allocated Resources:\n");
	for (int idx = 0; idx < customers; idx++) {
		output_print(resources_ptr[idx].allocated, resources);
	}

	printf("Need Resources:\n");
	for (int idx = 0; idx < customers; idx++) {
		output_print(resources_ptr[idx].need, resources);
	}
}

void execute_customers() {
	int *seq = (int*) malloc(customers * sizeof(int));
	int current_customer;

	/* ensure system is in safe state and run*/
	bool is_safe = safe_sequence(&seq);

	if (is_safe) {
		printf("Safe sequence is: ");
		output_print(seq, customers);

		for (int idx = 0; idx < customers; idx++) {
			current_customer = seq[idx];

			printf("--> Customer/Thread %d\n", current_customer);
			printf("    Allocated resources: ");
			output_print(resources_ptr[current_customer].allocated, resources);

			printf("    Needed: ");
			output_print(resources_ptr[current_customer].need, resources);

			printf("    Available: ");
			output_print(resources_arg, resources);

			printf("    Thread has started\n");
			resource_request(current_customer,
					resources_ptr[current_customer].need);
			printf("    Thread has finished\n");
			printf("    Thread is releasing resources\n");
			resource_release(current_customer,
					resources_ptr[current_customer].allocated);
			printf("    New available: ");
			output_print(resources_arg, resources);
		}
	} else
		printf("There is no safe sequence\n");
}

char* resource_request(int customer_number, int *request) {

	/* resource request algorithm */
	/* determines if system can fulfill request and be safe*/

	bool is_valid = true;
	int customer_number_cp = customer_number;

	/* request < customer need*/
	for (int idx = 0; idx < resources && is_valid; idx++) {
		is_valid = request[idx] <= resources_ptr[customer_number_cp].need[idx];
	}

	if (is_valid) {
		/* request < available so customer should wait till resources available */
		for (int idx = 0; idx < resources && is_valid; idx++) {
			is_valid = request[idx] <= resources_arg[idx];
		}
		if (is_valid) {
			for (int idx = 0; idx < resources; idx++) {
				resources_arg[idx] -= request[idx];
				resources_ptr[customer_number_cp].allocated[idx] +=
						request[idx];
				resources_ptr[customer_number_cp].need[idx] -= request[idx];
			}
			if (safe_sequence(NULL)) {
				return "State is safe, and request is satisfied\n";
			} else {
				/* if state becomes unsafe during */
				for (int idx = 0; idx < resources; idx++) {
					resources_arg[idx] += request[idx];
					resources_ptr[customer_number_cp].allocated[idx] -=
							request[idx]; //will
					resources_ptr[customer_number_cp].need[idx] += request[idx];
				}
				return "Resources not available for request and system state is unsafe \n";
			}
		} else {
			return " please wait, resources unavailable\n";
		}
	} else {
		return "request > maximum resource claim therefore unable to satisfy\n";
	}
}

char* resource_release(int customer_number, int *request) {
	/* resource release algorithm */
	/* basically releases resources from customer lol */

	int customer_number_cp = customer_number;
	bool is_valid = true;

	/* makes sure release doesn't create new resource */
	for (int idx = 0; idx < resources; idx++) {
		if (request[idx] > resources_ptr[customer_number_cp].allocated[idx]) {
			is_valid = false;
		}

	}
	if (is_valid) {
		/* make resource available */
		for (int idx = 0; idx < resources; idx++) {
			resources_arg[idx] += request[idx]; //will
		}
		return "The resources have been released successfully\n";
	} else
		return "Resource not in use. Can not release\n";
}

char* request_handling(char *user_input, int size, char* (*func)(int, int*)) {
	/* variable declaration */
	int customer_number = -1;
	int *user_request = (int*) malloc(size * sizeof(int));
	int counter = 0;
	char *input;

	int is_numeric;
	bool is_number = true;
	bool is_valid = true;

	/* parse user command */

	input = strsep(&user_input, " ");

	while (is_valid && (input = strsep(&user_input, " ")) != NULL) {

		/* make sure token is a number*/
		is_numeric = strlen(input);

		for (int idx = 0; idx < is_numeric && is_number; idx++) {
			is_number = input[idx] >= '0' && input[idx] <= '9';
		}

		if (is_number) {

			if (customer_number == -1) {
				if (atoi(input) >= 0)
					if (atoi(input) < customers)
						customer_number = atoi(input);
					/* invalid input case */
					else {
						free(user_request);
						is_valid = false;
						return "Error: customer number larger than expected\n";
					}
				/* invalid input case*/
				else {
					free(user_request);
					is_valid = false;
					return "Error: negative argument\n";
				}

			} else {
				if (counter < resources) {
					/*if input is valid store in user request array*/
					if (atoi(input) >= 0) {
						user_request[counter] = atoi(input);
						counter++;
					}
					/* invalid input cases*/
					else {
						free(user_request);
						is_valid = false;
						return "Error: negative argument \n";
					}
				} else { //will
					free(user_request);
					is_valid = false;
					return "Error: too many arguments\n";
				}
			}
		} else {
			free(user_request);
			is_valid = false;
			return "Error: non-numeric argument\n";
		}
	}

	/* if we get a valid request call the requested function*/

	if (is_valid) {
		if (counter == resources) {
			char *message = func(customer_number, user_request);
			free(user_request);
			return message;
		}
		/* invalid input cases*/
		else {
			free(user_request);
			is_valid = false;
			return "Error: arguments not enough\n";
		}
	}

	return "";

}

bool safe_sequence(int *seq[]) {
	int *work = (int*) malloc(resources * sizeof(int));
	bool *finish = (bool*) malloc(customers * sizeof(bool));
	int counter = 0;

	/* put starting values in work and finish */
	for (int idx = 0; idx < resources; idx++) {
		work[idx] = resources_arg[idx];
	}

	for (int idx = 0; idx < customers; idx++) {
		finish[idx] = false;
	}

	bool wait;
	bool is_safe;

	/* loop till system is safe or no changes made while unsafe*/
	for (int idx = 0; idx < customers; idx++) {
		is_safe = true;

		/* loop till we find a customer that finishes */
		for (int i = 0; i < customers; i++) {
			/* if customer is not finished */
			if (finish[i] == false) {
				wait = false;
				for (int j = 0; j < resources && !wait; j++) {
					if (resources_ptr[i].need[j] > work[j])
						wait = true;
				}
				/* if customer is fine add num to seq and move on*/
				if (!wait) {
					if (seq)
						(*seq)[counter++] = i;

					/* do updates */
					for (int r = 0; r < resources; r++)
						work[r] += resources_ptr[i].allocated[r];
					finish[i] = true;
				}
			}
		}

		/* if all customers are done then we can assume system is safe*/
		for (int idx = 0; idx < customers; idx++) {
			is_safe = is_safe && finish[idx];
		}
	}

	/* set sequence to null if there is no safe sequence*/
	if (!is_safe) {
		if (seq) {
			*seq = NULL;
			free(*seq);
		}
	}

	free(finish);
	free(work);

	return is_safe;
}

int* string_to_array(char *delimeter, char *message, int length) {
	int *array = (int*) malloc(length * sizeof(int));

	char *token;
	int idx = 0;

	while ((token = strsep(&message, delimeter)) != NULL) {
		if (strcmp(token, "") != 0) {
			array[idx] = atoi(token);
			idx++;
		}
	}
	return array;
}

void output_print(int *array, int length) {
	int pos = length - 1;

	for (int idx = 0; idx < length; idx++) {
		printf("%d", array[idx]);

		idx != pos ? printf(" ") : printf("\n");
	}
}
