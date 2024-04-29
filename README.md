# FAQ Chatbot

This project involves implementing a FAQ chatbot feature that can be turned on or off by clients. The chatbot provides responses to frequently asked questions (FAQs) based on predefined rules and it is improved by using a pre-trained GPT-2 language model.

## Task 1: Simple FAQ Chatbot

### FAQ Handling:
- Implement a set of frequently asked questions (FAQs) and their corresponding answers.
- When the chatbot is active, if a client's message matches any FAQ, the server responds with the corresponding answer.
- If no matching FAQ is found, the server responds with a default message "System Malfunction, I couldn't understand your query."

### Chatbot Activation/Deactivation:
- Clients can toggle the chatbot feature using commands:
  - `/chatbot login`: Avail the chatbot feature
  - `/chatbot logout`: Disable the feature
- The server maintains the chatbot status for each client.
- When clients avail the chatbot feature, the conversation continues using two prompts: "stupidbot>" for messages from the chatbot and "user>" for messages from the user.
- Appropriate messages are displayed on login and logout.

`compile using "gcc -o server server.c -luuid" the server.c  ant then "./server"
for client .c -> "gcc -o client client.c" and then "./client"`

#####NOTE#######\
 use space " " after each query\
################

## Task 2: FAQ Chatbot with GPT-2 (Improved)

### FAQ Handling:
- When the chatbot is active, the server responds with the corresponding answer using a pre-trained GPT-2 Large Language Model.
- There is no need to train the GPT-2 LLM using any custom data. Utilize the already pre-trained model to get the response.

### Chatbot Activation/Deactivation:
- Clients can toggle the chatbot feature using commands:
  - `/chatbot_v2 login`: Avail the chatbot feature
  - `/chatbot_v2 logout`: Disable the feature
- The server maintains the chatbot status for each client.
- When clients avail the chatbot feature, the conversation continues using two prompts: "gpt2bot>" for messages from the chatbot and "user>" for messages from the user.
- Appropriate messages are displayed on login and logout.

`run using "gcc -o server server.c -luuid" the server.c  ant then "./server"
for client .c -> "gcc -o client client.c" and then "./client"`

Feel free to reach out if you have any questions or need further assistance!
