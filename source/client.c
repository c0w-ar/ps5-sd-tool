#include "client.h"

int client = 0; // Global for other files

int wait_client(void) {

    int sock_listen = -1;
    int ret;
    struct sockaddr_in app_addr;

    // Open socket for a client
    sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_listen < 0) {
        notify("ps5-sd-tool\nFailed to open socket\n");
        return 1;
    }
    const int enable = 1;
    ret = setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (ret < 0) {
        notify("ps5-sd-tool\nFailed to setsockopt\n");
        return 1;
    }

    memset(&app_addr, 0, sizeof(app_addr));
    app_addr.sin_family = AF_INET;
    app_addr.sin_len = sizeof(app_addr);
    app_addr.sin_port = htons(APP_PORT);
    app_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(sock_listen, (const struct sockaddr *)&app_addr, sizeof(app_addr));
    if (ret < 0) {
        notify("ps5-sd-tool\nFailed to bind\n");
        return 1;
    }
    ret = listen(sock_listen, 5);
    if (ret < 0) {
        notify("ps5-sd-tool\nFailed to listen\n");
        return 1;
    }
    else {
        notify("ps5-sd-tool\nListening for client on port: %d\n", APP_PORT);
    }

    client = accept(sock_listen, 0, 0);

    close(sock_listen);

    return 0;
}

void close_client(void) {
    shutdown(client, SHUT_WR);
    close(client);
}

void printf_s(const char *format, ...) {
    char buffer[1024];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        write(client, buffer, len);
    }
}

char read_char(void) {
    char c;
    char lf;

    while(1) {
        int len = read(client, &c, 1);
        if(len == -1 && errno == EINTR || c == 0)
            continue;
        if(c == 0x0A || c == 0x0D)   // Return on "empty" input (only LF)
            return c;
        else
            break;
    }
    read(client, &lf, 1); // Read lf to discard it
    return c;
}

int read_n_digits(void* buf, int n_digits) {
    int n_bytes = read(client, buf, n_digits+1); // We account for final LF
    return n_bytes -1; // Return input digits less LF
}

uint64_t read_uint64(void) {

    uint8_t buffer[17];
    memset(buffer, 0, 17);

    int n_bytes = read_n_digits(buffer, 16);
    buffer[n_bytes] = '\0'; // Remove last LF value

    return strtoull((char*)buffer, NULL, 16);
}

uint32_t read_uint32(void) {

    uint8_t buffer[9];
    memset(buffer, 0, 9);

    int n_bytes = read_n_digits(buffer, 8);
    buffer[n_bytes] = '\0'; // Remove last LF value

    return strtoul((char*)buffer, NULL, 16);
}