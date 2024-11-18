#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILE_NAME "arqurivo"

void send_file(int client_socket) {
    FILE *file = fopen(FILE_NAME, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Obter o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Enviar o tamanho do arquivo ao cliente
    uint32_t net_file_size = htonl(file_size); // Converter para a ordem de bytes de rede
    send(client_socket, &net_file_size, sizeof(net_file_size), 0);

    printf("Tamanho do arquivo (%zu bytes) enviado ao cliente.\n", file_size);

    // Enviar o arquivo em partes
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        size_t bytes_sent = 0;
        while (bytes_sent < bytes_read) {
            ssize_t result = send(client_socket, buffer + bytes_sent, bytes_read - bytes_sent, 0);
            if (result == -1) {
                perror("Erro ao enviar dados");
                fclose(file);
                return;
            }
            bytes_sent += result;
        }
    }

    fclose(file);

    // Indicar fim da transmissão
    shutdown(client_socket, SHUT_WR);
    printf("Arquivo enviado com sucesso.\n");
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao associar o socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Erro ao escutar no socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escutando na porta %d...\n", PORT);

    if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
        perror("Erro ao aceitar conexão");
        close(server_fd);
        exit(EXIT_FAILURE);
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
        printf("Mensagem incorreta enviada ao cliente.\n");
    }

    close(client_socket);
    close(server_fd);
    printf("Conexão encerrada.\n");

    return 0;
}
