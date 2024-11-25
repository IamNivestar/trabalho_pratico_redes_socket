#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8081
#define BUFFER_SIZE 4 * 1024
#define FILENAME "arquivo"

int main()
{
    FILE *arquivo = fopen(FILENAME, "rb");

    if (!arquivo) 
    {
        printf("\nErro ao abrir arquivo\n");
        return 0;
    }

    int socketUdp;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE + sizeof(uint32_t)];
    socklen_t client_len;
    uint32_t seq_num = 0; // Número de sequência do pacote

    if ((socketUdp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Erro ao criar socket\n");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (bind(socketUdp, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erro ao fazer bind\n");
        return 0;
    }

    printf("Servidor UDP aguardando mensagens na porta %d...\n", PORT);

    client_len = sizeof(client_addr);

    if (recvfrom(socketUdp, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len) < 0)
    {
        printf("Erro ao receber mensagem\n");
    }

    printf("Ok, recebi uma solicitação: %s\n", buffer);
    
    size_t bytes_read;
    while ((bytes_read = fread(buffer + sizeof(uint32_t), 1, BUFFER_SIZE, arquivo)) > 0)
    {
        // Adicionar número de sequência no início do pacote
        *(uint32_t *)buffer = htonl(seq_num);

        if (sendto(socketUdp, buffer, bytes_read + sizeof(uint32_t), 0, (struct sockaddr *)&client_addr, client_len) < 0)
        {
            printf("Erro ao enviar dados\n");
        }

        seq_num++;
    }

    memset(buffer, 0, BUFFER_SIZE);
    if (sendto(socketUdp, buffer, 0, 0, (struct sockaddr *)&client_addr, client_len) < 0)
    {
        printf("Erro ao enviar pacote de término\n");
    }

    close(socketUdp);
    return 0;
}
