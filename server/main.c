#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <xsdk/net.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

static const char PAGE[] =
    "<!doctype html><html lang='pt-BR'>"
    "<head><meta charset='utf-8'><title>Xenon Server</title></head>"
    "<body><h1>Xenon Browser Server</h1>"
    "<p>Servidor HTTP rodando no Xbox 360!</p>"
    "</body></html>";

static int send_all(int fd, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t result = send(fd, data + sent, length - sent, 0);
        if (result <= 0)
            return -1;
        sent += (size_t)result;
    }

    return 0;
}

int main(void)
{
    uint32_t link = 0;
    struct in_addr address;

    printf("Xenon Server iniciando...\n");

    if (xsdk_net_link(&link) != 0 ||
        !(link & XSDK_NET_LINK_ACTIVE)) {
        printf("Rede indisponivel.\n");
        return 1;
    }

    if (xsdk_net_address(&address) != 0) {
        printf("IP indisponivel.\n");
        return 1;
    }

    printf("IP: %u.%u.%u.%u\n",
        (unsigned)((address.s_addr >> 24) & 255),
        (unsigned)((address.s_addr >> 16) & 255),
        (unsigned)((address.s_addr >> 8) & 255),
        (unsigned)(address.s_addr & 255));

    int server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) {
        printf("Erro socket: %d\n", errno);
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Erro bind: %d\n", errno);
        close(server);
        return 1;
    }

    if (listen(server, 8) < 0) {
        printf("Erro listen: %d\n", errno);
        close(server);
        return 1;
    }

    printf("Servidor ativo na porta %d!\n", SERVER_PORT);

    while (1) {
        int client = accept(server, NULL, NULL);

        if (client < 0) {
            printf("Erro accept: %d\n", errno);
            continue;
        }

        char request[BUFFER_SIZE];
        ssize_t received = recv(client, request, sizeof(request) - 1, 0);

        if (received > 0) {
            request[received] = '\0';

            char header[256];
            int length = snprintf(
                header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: %u\r\n"
                "Connection: close\r\n\r\n",
                (unsigned)(sizeof(PAGE) - 1));

            if (length > 0 && (size_t)length < sizeof(header)) {
                send_all(client, header, (size_t)length);
                send_all(client, PAGE, sizeof(PAGE) - 1);
            }
        }

        close(client);
    }

    close(server);
    return 0;
}
