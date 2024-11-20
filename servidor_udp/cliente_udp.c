#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define FILENAME "arquivo"
#define PORT 80
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    printf("\nIniciando cliente UDP...\n");
    FILE *arquivo = fopen(FILENAME, "rb");

    if (!arquivo) 
    {
        printf("\nErro ao abrir arquivo");
        return 0;
    }

    int socketUdp = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
 
    if (bind(socketUdp, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erro ao associar o socket ao endereço e porta");
        return 0;
    }

    printf("Servidor UDP aguardando cliente na porta %d", PORT);

    char buffer[BUFFER_SIZE];
    if (recvfrom(socketUdp, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len) < 0)
    {
        printf("Erro ao receber mensagem do cliente");
    }

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

    printf("Envio do arquivo concluído.\n");

    // Fechando o arquivo e o socket
    fclose(arquivo);
    close(socketUdp);

    return 0;
}