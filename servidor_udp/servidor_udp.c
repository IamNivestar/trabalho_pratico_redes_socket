#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILENAME "teste.txt"

int main()
{
    FILE *arquivo = fopen(FILENAME, "rb");

    if (!arquivo) 
    {
        printf("\nErro ao abrir arquivo");
        return 0;
    }

    int socketUdp;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_len;

    if ((socketUdp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Erro ao criar socket");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (bind(socketUdp, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erro ao fazer bind");
        return 0;
    }

    printf("Servidor UDP aguardando mensagens na porta %d...\n", PORT);

    // client_len = sizeof(client_addr);

    if (recvfrom(socketUdp, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len) < 0)
    {
        printf("Erro ao receber mensagem");
        // return 0;
    }

    printf("Ok, recebi uma solicitacao: %s", buffer);
    
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, arquivo)) > 0)
    {
        if (sendto(socketUdp, buffer, bytes_read, 0, (struct sockaddr *)&client_addr, client_len) < 0)
        {
            printf("Erro ao enviar dados");
        }
    }

    memset(buffer, 0, BUFFER_SIZE);
    if (sendto(socketUdp, buffer, 0, 0, (struct sockaddr *)&client_addr, client_len) < 0)
    {
        printf("Erro ao enviar pacote de término");
    }

    close(socketUdp);
    return 0;
}
