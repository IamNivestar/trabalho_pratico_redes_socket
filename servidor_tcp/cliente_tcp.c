#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define OUTPUT_FILE "arquivo_transferido"

// Função para calcular e exibir a taxa de download
void calculate_download_speed(size_t total_bytes, double duration) {
    double speed = total_bytes / duration;
    printf("Taxa de download: %.2f bytes/segundo\n", speed);
}

// Função para receber o arquivo
int receive_file(int client_socket, size_t expected_size) {
    FILE *file = fopen(OUTPUT_FILE, "wb");
    if (!file) {
        perror("Erro ao criar arquivo de saída");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t total_bytes = 0;
    ssize_t bytes_received;
    clock_t start_time = clock();

    // Receber o arquivo do servidor
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
        total_bytes += bytes_received;
    }

    fclose(file);
    clock_t end_time = clock();
    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (bytes_received < 0) {
        perror("Erro ao receber dados");
        return -1;
    }

    printf("Arquivo recebido com sucesso. %zu bytes recebidos.\n", total_bytes);
    calculate_download_speed(total_bytes, duration);

    if (total_bytes != expected_size) {
        printf("Erro: o tamanho recebido (%zu bytes) não corresponde ao esperado (%zu bytes).\n", total_bytes, expected_size);
        return -1;
    }

    printf("Arquivo validado: todos os bytes foram recebidos corretamente.\n");
    return 0;
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Alterar para o IP do servidor, se necessário
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Endereço inválido");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Digite uma mensagem para enviar ao servidor: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0;  // Remove o '\n'

    send(client_socket, buffer, strlen(buffer), 0);

    // Receber status do servidor
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (strcmp(buffer, "OK") == 0) {
        printf("Mensagem aceita pelo servidor. Recebendo tamanho do arquivo...\n");

        // Receber o tamanho do arquivo
        uint32_t file_size;
        recv(client_socket, &file_size, sizeof(file_size), 0);
        file_size = ntohl(file_size); // Converter para a ordem de bytes do host

        printf("Tamanho esperado do arquivo: %u bytes.\n", file_size);

        // Receber o arquivo
        if (receive_file(client_socket, file_size) == 0) {
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
