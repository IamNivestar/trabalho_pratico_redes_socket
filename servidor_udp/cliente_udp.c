#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define FILENAME "arquivo"
#define OUTPUT "arquivo_recebido_do_servidor"
#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    printf("\nIniciando cliente UDP...\n");

    int socketUdp = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    const char *mensagem = "Iai cara, me manda o arquivo";
    if (sendto(socketUdp, mensagem, strlen(mensagem), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erro ao enviar solicitação");
    }

    FILE *arquivo = fopen(OUTPUT, "wb");
    if (!arquivo)
    {
        printf("Erro ao abrir arquivo para gravação");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recvfrom(socketUdp, buffer, BUFFER_SIZE, 0, NULL, NULL)) > 0)
    {
        if (bytes_received == 0)
        {
            break;
        }
        fwrite(buffer, 1, bytes_received, arquivo);
    }

    printf("Fim...");

    fclose(arquivo);
    close(socketUdp);

    return 0;
}