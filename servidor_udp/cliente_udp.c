#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define OUTPUT "arquivo_transferido"
#define PORT 8081
#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
    clock_t temp_ini = clock();
    printf("\nIniciando cliente UDP...\n");

    int socketUdp = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    char *mensagem = "Iai cara, me manda o arquivo";
    if (sendto(socketUdp, mensagem, strlen(mensagem), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erro ao enviar solicitação\n");
    }

    FILE *arquivo = fopen(OUTPUT, "wb");
    if (!arquivo)
    {
        printf("Erro ao abrir arquivo para gravação\n");
    }

    char buffer[BUFFER_SIZE + sizeof(uint32_t)];
    ssize_t bytes_received;
    uint32_t expected_seq_num = 0;
    size_t total_bytes = 0;
    int pacotes_perdidos = 0;

    
    while ((bytes_received = recvfrom(socketUdp, buffer, BUFFER_SIZE + sizeof(uint32_t), 0, NULL, NULL)) > 0)
    {
        if (bytes_received == 0)
        {
            break;
        }

        // obtendo numero de sequencia convertido para a ordem de bytes da rede
        uint32_t seq_num = ntohl(*(uint32_t *)buffer);

        // conta pacote perdido
        if (seq_num != expected_seq_num)
        {
            pacotes_perdidos += seq_num - expected_seq_num;
            expected_seq_num = seq_num;
        }

        //gravando
        fwrite(buffer + sizeof(uint32_t), 1, bytes_received - sizeof(uint32_t), arquivo);
        total_bytes += bytes_received - sizeof(uint32_t);

        expected_seq_num++;
    }

    printf("\nFim da transferência...");
    long tamanho_do_arquivo_gravado = ftell(arquivo);
    fclose(arquivo);
    close(socketUdp);

    clock_t temp_end = clock();
    double temp_total = (double)(temp_end - temp_ini) / CLOCKS_PER_SEC;
    double taxa_download = tamanho_do_arquivo_gravado / temp_total;

    printf("\nTaxa de download: %.2f \n", taxa_download);
    FILE *file = fopen("velocidades.txt", "a");
    if (!file) {
        perror("Erro ao abrir o arquivo de velocidades");
        return -1;
    }

    fprintf(file, "%2f;", taxa_download);
    fclose(file);
    printf("Total de pacotes perdidos: %d\n", pacotes_perdidos);

    FILE *file2 = fopen("pacotes_lost.txt", "a");
    if (!file2) {
        perror("Erro ao abrir o arquivo de velocidades");
        return -1;
    }

    fprintf(file2, "%d;", pacotes_perdidos);
    fclose(file2);

    return 0;
}
