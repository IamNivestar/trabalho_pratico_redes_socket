#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILE_NAME "arquivo"

void send_file(int client_socket) {
    
    FILE *file = fopen(FILE_NAME, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file); //o ponteiro ta no final do arquivo depois do fseek, a posição do ftell é o tamanho do arquivo
    fseek(file, 0, SEEK_SET); // tive q voltar o ponteiro para o inicio do arquivo pq começarei a transferir o arquivo

    uint32_t net_file_size = htonl(file_size); // Converter para a ordem de bytes de rede
    send(client_socket, &net_file_size, sizeof(net_file_size), 0);  // enviando o tamanho do arquivo ao cliente

    printf("Tamanho do arquivo (%zu bytes) enviado ao cliente.\n", file_size);

    // Enviar os dados do  arquivo de forma parcionada em pacotes
    char buffer[BUFFER_SIZE]; //criando buffer do tamanho do pacote escolhido
    size_t bytes_read; 

    size_t bytes_to_read = BUFFER_SIZE; 

    while (1) {

        bytes_read = fread(buffer, 1, bytes_to_read, file); //obtendo um pacote do arquivo e salvando no buffer

        if (bytes_read <= 0) { 
            break; 
        }

        size_t bytes_sent = 0; 

        while (bytes_sent < bytes_read) {
            size_t bytes_remaining = bytes_read - bytes_sent; // quanto ainda ta faltando enviar

            ssize_t result = send(client_socket, buffer + bytes_sent, bytes_remaining, 0); //enviar pacote
            if (result == -1) {
                perror("Erro ao enviar dados\n");
                fclose(file);
                return;
            }

            bytes_sent += result; 
        }
    }


    fclose(file);
    shutdown(client_socket, SHUT_WR); // avisando que a transmissao de dados acabou
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

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    struct sockaddr *server_addr_ptr = (struct sockaddr *)&server_addr;
    int bin_result = bind(server_fd, server_addr_ptr, sizeof(server_addr));

    if (bin_result < 0) {
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
    }
    else{  
        printf("Cliente Conectado\n");
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
