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


#define PORT 5554
#define SERVER_IP "127.0.0.1"
#define MAX_CLIENTS 10
#define MAX_FAQS 10000

typedef struct {
    char question[1024];
    char answer[1024];
} FAQ;

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

FAQ faqs[MAX_FAQS];
int num_faqs = 0;
Client clients[MAX_CLIENTS];

// Function to load FAQs
void load_faqs(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[2048];
    while (fgets(line, sizeof(line), file)) {
        char *question = strtok(line, "|||");
        // printf(question);
        char *answer = strtok(NULL, "|||");
        // printf(answer);
        if (question != NULL && answer != NULL && num_faqs < MAX_FAQS) {
            strcpy(faqs[num_faqs].question, question);
            strcpy(faqs[num_faqs].answer, answer);
            // printf("Question: %s\n", faqs[num_faqs].question);
            // printf("Answer: %s\n", faqs[num_faqs].answer);
            num_faqs++;
            }
    }

    fclose(file);
}

// Function to handle client's message
// Check if the message matches any FAQ question
void handle_message(int client_index, const char *message) {
    if (clients[client_index].chatbot_active) {
        
        bool found = false;
        for (int i = 0; i <num_faqs; i++) {
            if (strcmp(message, faqs[i].question) == 0) {
                char response[2048];
                snprintf(response, sizeof(response), "stupidbot> %s", faqs[i].answer);
                send(clients[client_index].socket, response, strlen(response), 0);
                found = true;
                break;
            }
        }
        if (!found) {
            send(clients[client_index].socket, "stupidbot> System Malfunction, I couldn't understand your query.", strlen("stupidbot> System Malfunction, I couldn't understand your query."), 0);
        }
    } else {
        send(clients[client_index].socket, "stupidbot> FAQ chatbot is currently disabled.", strlen("stupidbot> FAQ chatbot is currently disabled."), 0);
    }
}

// Function to handle chatbot activation/deactivation
void handle_chatbot(int client_index, const char *message) {
    if (strncmp(message, "/chatbot login", 14) == 0) {
        clients[client_index].chatbot_active = true;
        send(clients[client_index].socket, "stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions", strlen("stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions"), 0);
    } else if (strncmp(message, "/chatbot logout", 15) == 0) {
        clients[client_index].chatbot_active = false;
        send(clients[client_index].socket, "stupidbot> Bye! Have a nice day and do not complain about me", strlen("stupidbot> Bye! Have a nice day and do not complain about me"), 0);
    }else {
    	send(clients[client_index].socket, "stupidbot> Invalid command !!", strlen("stupidbot> Invalid command !!"), 0);
    }
}

// Function to print UUID 
void print_uuid(const uuid_t uuid) {
    char uuid_str[37]; // UUIDs are 36 characters long plus '\0'
    uuid_unparse(uuid, uuid_str);
    printf("%s", uuid_str);
}

int main() {
    int server_socket, client_socket, max_sd, activity;
    struct sockaddr_in server_addr, client_addr;
    fd_set read_fds;
    char message[1024];

    // Load FAQs from file
    load_faqs("FAQs.txt");

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
                    print_uuid(clients[i].uuid); 
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
