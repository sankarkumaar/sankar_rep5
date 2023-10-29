#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>


#define PORT_AVAILABILITY 8000
#define PORT_MESSAGES 9000

#define MAX_CLIENTS 100

struct Client {
    char name[50];
    struct sockaddr_in address;
    int client_port;
    char client_ip[50];

};

struct Client available_clients[MAX_CLIENTS];
int num_clients = 0;

int main() {
    int res;
    int availability_socket, messages_socket;


//    struct sockaddr_in client_addr;


    printf("server starts running");//modified 1
    struct sockaddr_in server_addr,client_address;
    struct pollfd fds[2];

    socklen_t addr_len = sizeof(client_address);
    availability_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if(availability_socket < 0) {

     perror("availability_socket() creation failed : ");
     exit(EXIT_FAILURE);

    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT_AVAILABILITY);

    res =  bind(availability_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(res < 0) {

       perror("availability_socket() bind failed :");       
       exit(EXIT_FAILURE);

    }


    messages_socket = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(messages_socket < 0) {

     perror("messages_socket() creation failed : ");
     exit(EXIT_FAILURE);

    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT_MESSAGES);

    res = bind(messages_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(res < 0) {

       perror("messages_socket() bind failed :");
	   close(availability_socket);
       close(messages_socket);
       exit(EXIT_FAILURE);
    }


    fds[0].fd = availability_socket;
    fds[0].events = POLLIN;

    fds[1].fd = messages_socket;
    fds[1].events = POLLIN;

//    memset(available_clients, 0, sizeof(available_clients));
    
    while (1) {

        int poll_status = poll(fds, sizeof(fds)/sizeof(fds[0]), -1);// wait untill an event is occurred


        if (poll_status == -1) {
            perror("Poll Failed :");
            exit(EXIT_FAILURE);
        }


        if (fds[0].revents & POLLIN) {


            char client_name[50];
            char client_ip[50];
            int client_port;
            int client_exists = 0;
            struct sockaddr_in client_address;
            socklen_t cliaddr_len = sizeof(client_address);

            recvfrom(availability_socket, client_name, sizeof(client_name), 0, (struct sockaddr *)&client_address, &cliaddr_len); //


            inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
            client_port = ntohs(client_address.sin_port);

            printf("%s,%d\n",client_ip,client_port);
            

            for (int i = 0; i < num_clients; ++i) {

                if (strcmp(available_clients[i].client_ip, client_ip) == 0 ) {

                      client_exists = 1;
                      printf("Client already in the list\n");
                      break;
                }
            }

            if (!client_exists && num_clients < MAX_CLIENTS) {
                strcpy(available_clients[num_clients].name, client_name);
                strcpy(available_clients[num_clients].client_ip, client_ip);
                available_clients[num_clients].address = client_address;
                available_clients[num_clients].client_port =  client_port;

                num_clients++;

               printf(" struct %d\n", client_address);
            }
            else if(!client_exists && num_clients == MAX_CLIENTS) {

                   printf("Client count reached its maximum allocated size");

            }

        }


        if (fds[1].revents & POLLIN) {

            char buffer[1024];
            char sender_name[50];
			char client_ip[50];
            char name[50];
            int bytestoread;
            socklen_t cliaddr_len = sizeof(client_address);//
            struct sockaddr_in client_address;

            memset(buffer, 0, sizeof(buffer));
            bytestoread = recvfrom(messages_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &cliaddr_len);
			inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);

            if(bytestoread < 0) {
                 perror(" Reception failed: ");
                 exit(EXIT_FAILURE);
            }
            buffer[bytestoread]='\0';
            printf("Received message from %s: %s, %d \n", sender_name, buffer, num_clients );
            printf(" struct %d,%d\n", client_address.sin_family,client_address.sin_port);


            for (int i = 0; i < num_clients; ++i) {
                 printf("avail port : %d, %s \n",available_clients[i].client_port,available_clients[i].client_ip);

                 if (available_clients[num_clients].client_ip != client_ip) {//

                    //socklen_t cliaddr_len = sizeof(available_clients[i].address);
                     printf(" struct2 %d\n", available_clients[i].address);
                     res = sendto(messages_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&available_clients[i].address, sizeof(available_clients[i].address));

                     if(res < 0) {
                       perror(" Sending failed ");
                       exit(EXIT_FAILURE);
                     }

                }
           }


        }
    }


    close(availability_socket);
    close(messages_socket);

    return 0;
}
