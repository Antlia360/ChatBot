#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdbool.h>
#include <ctype.h> 
#include <uuid/uuid.h>



#define PORT 5555
#define SERVER_IP "127.0.0.1"
#define MAX_CLIENTS 10
#define MAX_RESPONSE_SIZE 5000


// Structure to hold client information
typedef struct {
    uuid_t uuid;
    int socket;
    bool chatbot_active;
} Client;

// Function to generate UUID
void generate_uuid(uuid_t uuid) {
    uuid_generate(uuid);
}


Client clients[MAX_CLIENTS];


// Function to print UUID 
// void print_uuid(const uuid_t uuid) {
//     char uuid_str[37]; // UUIDs are 36 characters long plus '\0'
//     uuid_unparse(uuid, uuid_str);
//     printf("%s", uuid_str);
// }
char* print_uuid(const uuid_t uuid) {
    static char uuid_str[37]; // UUIDs are 36 characters long plus '\0'
    uuid_unparse(uuid, uuid_str);
    for (int i = 0; uuid_str[i]; i++) {
        if (uuid_str[i] == '-') {
            // Shift characters to the left to remove the hyphen
            for (int j = i; uuid_str[j]; j++) {
                uuid_str[j] = uuid_str[j + 1];
            }
        }
    }
    return uuid_str;
}
// Function to handle client's message
// Check if the message matches any FAQ question
void handle_message(int client_index, const char *message) {

    if (clients[client_index].chatbot_active) {
        // const char *prompt = "\"Hi how are you ?\"";
        // const char *output_file = "output.txt";
        char command[5000];
        char output_file[100];
        // printf("python3 gpt_2_gen.py %s %s \n",message, print_uuid(clients[client_index].uuid));
        sprintf(command, "python3 gpt_2_gen.py %s %s ",message, print_uuid(clients[client_index].uuid));

        // Execute the command using system call
        int ret = system(command);

        if (ret == -1) {
        // system call failed
            perror("system call failed");
            return 1;
        } else {
            // system call succeeded
            printf("Python script executed with return code: %d\n", ret);
            // Open the output file for reading
            sprintf(output_file, "%s.txt", print_uuid(clients[client_index].uuid));
            FILE *file = fopen(output_file, "r");
            if (file == NULL) {
                perror("Error opening text file");
                return 1;
            }

            // Read the contents of the file
            char response[MAX_RESPONSE_SIZE];
            size_t bytes_read = fread(response, sizeof(char), MAX_RESPONSE_SIZE, file);
            if (bytes_read == 0) {
                perror("Error reading file");
                fclose(file);
                return 1;
            }
            // Close the file
            fclose(file);
            // Ensure response is null-terminated
            response[bytes_read] = '\0';
            // Prepend "gpt2bot> " to the response
            char formatted_response[MAX_RESPONSE_SIZE + 12];
            snprintf(formatted_response, sizeof(formatted_response), "gpt2bot> %s", response);
            send(clients[client_index].socket, formatted_response, strlen(formatted_response), 0);


        }
    } else {
        send(clients[client_index].socket, "gpt2bot> FAQ chatbot is currently disabled.", strlen("gpt2bot> FAQ chatbot is currently disabled."), 0);
    }
}

// Function to handle chatbot activation/deactivation
void handle_chatbot(int client_index, const char *message)  {
    if (strncmp(message, "/chatbot_v2 login", 17) == 0) {
        // memcpy(clients[client_index].uuid, client_uuid, sizeof(uuid_t));
        printf(print_uuid(clients[client_index].uuid)); 
        printf("  Client logged_in to GPT2BOT ! \n");
        clients[client_index].chatbot_active = true;
        send(clients[client_index].socket, "gpt2bot> Hi, I am updated bot, I am able to answer any question be it correct or incorrect", strlen("gpt2bot> Hi, I am updated bot, I am able to answer any question be it correct or incorrect"), 0);
    } else if (strncmp(message, "/chatbot_v2 logout", 18) == 0) {
        printf(print_uuid(clients[client_index].uuid)); 
        printf("  Client logged_out to GPT2BOT ! \n");
        clients[client_index].chatbot_active = false;
        send(clients[client_index].socket, "gpt2bot> Bye! Have a nice day and hope you do not have any complaints about me", strlen("gpt2bot> Bye! Have a nice day and hope you do not have any complaints about me"), 0);
    }else {
    	send(clients[client_index].socket, "gpt2bot> Invalid command !!", strlen("gpt2bot> Invalid command !!"), 0);
    }
}


int main() {
    int server_socket, client_socket, max_sd, activity;
    struct sockaddr_in server_addr, client_addr;
    fd_set read_fds;
    char message[1024];

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Initialize clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        clients[i].chatbot_active = false;
    }

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds); // Add server socket to the set
        max_sd = server_socket;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0) {
                FD_SET(clients[i].socket, &read_fds);
                if (clients[i].socket > max_sd) {
                    max_sd = clients[i].socket;
                }
            }
        }

        // Use select to monitor for activity
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("Select error\n");
        }

        // Check if there is incoming connection
        if (FD_ISSET(server_socket, &read_fds)) {
            // Accept the incoming connection
            socklen_t client_addr_len = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }
            // printf("New connection accepted\n");

            // Generate UUID for the new client
            uuid_t client_uuid;
            generate_uuid(client_uuid);

            // Add new client to the clients array
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = client_socket;
                    clients[i].chatbot_active = false;
                    memcpy(clients[i].uuid, client_uuid, sizeof(uuid_t));
                    printf("Client %d added with UUID: ", i);
                    printf(print_uuid(clients[i].uuid)); 
                   
                    printf("\n");// Print the UUID
                    break;
                }
            }
        }

        // Check if there is activity on client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            client_socket = clients[i].socket;
            if (FD_ISSET(client_socket, &read_fds)) {
                // Receive message from client
                int bytes_received = recv(client_socket, message, sizeof(message), 0);
                if (bytes_received <= 0) {
                    // Client disconnected
                    close(client_socket);
                    clients[i].socket = 0;
                    printf("Client %d disconnected\n", i);
                } else {
                    message[bytes_received] = '\0';
                    // Check if the message is related to chatbot activation/deactivation
                    if (strncmp(message, "/", 1) == 0) {
                        handle_chatbot(i, message);
                    } else{
                        // printf("Message : %s", message);
                        // Handle client's message
                        handle_message(i, message);
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}