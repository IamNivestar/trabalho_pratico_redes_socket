#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 512
#define FILE_NAME "arquivo"

void send_file(int client_socket) {
    FILE *file = fopen(FILE_NAME, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint32_t net_file_size = htonl(file_size);
    send(client_socket, &net_file_size, sizeof(net_file_size), 0);

    printf("Tamanho do arquivo (%zu bytes) enviado ao cliente.\n", file_size);

    char buffer[BUFFER_SIZE];
    uint32_t seq_num = 0; // Número de sequência inicial

    size_t bytes_read;
    while ((bytes_read = fread(buffer + sizeof(seq_num), 1, BUFFER_SIZE - sizeof(seq_num), file)) > 0) {
        seq_num = htonl(seq_num); // Converter para ordem de rede
        memcpy(buffer, &seq_num, sizeof(seq_num)); // Copiar o número de sequência no início do buffer

        size_t bytes_to_send = bytes_read + sizeof(seq_num);
        size_t bytes_sent = 0;

        while (bytes_sent < bytes_to_send) {
            ssize_t result = send(client_socket, buffer + bytes_sent, bytes_to_send - bytes_sent, 0);
            if (result == -1) {
                perror("Erro ao enviar dados\n");
                fclose(file);
                return;
            }
            bytes_sent += result;
        }

        seq_num = ntohl(seq_num) + 1; // Incrementar número de sequência para o próximo pacote
    }

    fclose(file);
    shutdown(client_socket, SHUT_WR);
    printf("Arquivo enviado com sucesso.\n");
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erro ao configurar SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao associar o socket\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Erro ao escutar no socket\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escutando na porta %d...\n", PORT);

    client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

    if (client_socket < 0) {
        perror("Erro ao aceitar conexão\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    } else {  
        printf("Cliente conectado\n");
    }

    memset(buffer, 0, BUFFER_SIZE); 
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    printf("Mensagem recebida: %s\n", buffer);

    if (strcmp(buffer, "send me the file") == 0) {
        printf("Mensagem correta. Enviando arquivo...\n");
        send(client_socket, "OK", 2, 0);
        send_file(client_socket);
    } else {
        send(client_socket, "ERROR", 5, 0);
        printf("Mensagem incorreta enviada pelo cliente.\n");
    }

    close(client_socket);
    close(server_fd);
    printf("Conexão encerrada.\n");

    return 0;
}

