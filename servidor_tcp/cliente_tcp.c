#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 4 * 1024
#define OUTPUT_FILE "arquivo_transferido"

// função para calcular e exibir a taxa de download
void calculate_download_speed(size_t total_bytes, double tempo) {
    double speed = total_bytes / tempo;
    printf("Taxa de download: %.2f bytes/segundo\n", speed);

    FILE *file = fopen("velocidades.txt", "a");
    if (!file) {
        perror("Erro ao abrir o arquivo de velocidades");
        return;
    }

    // Gravar a velocidade no arquivo
    fprintf(file, "%.2f;", speed);
    fclose(file);
}

//função para receber o arquivo
int receive_file(int client_socket, size_t tamanho_esperado, clock_t start_time) {
    FILE *file = fopen(OUTPUT_FILE, "wb");
    if (!file) {
        perror("Erro ao criar arquivo de saída");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t total_bytes = 0;
    ssize_t bytes_received;
    uint32_t expected_seq_num = 0; // Número de sequência esperado
    size_t lost_packets = 0;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        uint32_t received_seq_num;
        memcpy(&received_seq_num, buffer, sizeof(received_seq_num));
        received_seq_num = ntohl(received_seq_num);

        if (received_seq_num != expected_seq_num) {
            printf("Pacotes perdidos: esperava %u, recebi %u.\n", expected_seq_num, received_seq_num);
            lost_packets += (received_seq_num - expected_seq_num);
            expected_seq_num = received_seq_num; // Sincronizar o próximo número de sequência
        }

        fwrite(buffer + sizeof(received_seq_num), 1, bytes_received - sizeof(received_seq_num), file);
        total_bytes += bytes_received - sizeof(received_seq_num);
        expected_seq_num++;
    }

    fclose(file);
    clock_t end_time = clock();
    double tempo = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Arquivo recebido com sucesso. %zu bytes recebidos.\n", total_bytes);
    calculate_download_speed(total_bytes, tempo);

    if (lost_packets > 0) {
        printf("Total de pacotes perdidos: %zu.\n", lost_packets);
    } else {
        printf("Nenhum pacote perdido.\n");
    }

    FILE *file2 = fopen("pacotes_lost.txt", "a");
    if (!file2) {
        perror("Erro ao abrir o arquivo de velocidades");
        return -1;
    }

    fprintf(file2, "%zu;", lost_packets);
    fclose(file2);

    if (total_bytes != tamanho_esperado) {
        printf("Erro: o tamanho recebido (%zu bytes) não corresponde ao esperado (%zu bytes).\n", total_bytes, tamanho_esperado);
        return -1;
    }

    printf("Arquivo validado: todos os bytes foram recebidos corretamente.\n");
    return 0;
}

int main() {

    int client_socket;
    
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) { // Criando socket do cliente
        perror("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Endereço inválido\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Digite uma mensagem para enviar ao servidor (send me the file): \n");
    // fgets(buffer, BUFFER_SIZE, stdin);
    strcpy(buffer, "send me the file");
    buffer[strcspn(buffer, "\n")] = 0;  // Remove o \n

    clock_t start_time = clock();

    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (strcmp(buffer, "OK") == 0) {
        printf("Mensagem aceita pelo servidor. Recebendo tamanho do arquivo...\n");

        // Receber o tamanho do arquivo
        uint32_t file_size;
        recv(client_socket, &file_size, sizeof(file_size), 0);
        file_size = ntohl(file_size);

        printf("Tamanho esperado do arquivo: %u bytes.\n", file_size);

        // Receber o arquivo
        if (receive_file(client_socket, file_size, start_time) == 0) {
            printf("Download concluído com sucesso.\n");
        } else {
            printf("Erro no download do arquivo.\n");
        }
    } else if (strcmp(buffer, "ERROR") == 0) {
        printf("Mensagem rejeitada pelo servidor.\n");
    } else {
        printf("Resposta inesperada do servidor: %s\n", buffer);
    }

    close(client_socket);
    return 0;
}
