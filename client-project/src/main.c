#include "protocol.h"

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); // Disabilita buffering output

    char server_ip[CITY_LEN] = DEFAULT_IP;
    int port = DEFAULT_PORT;
    weather_request_t req;
    memset(&req, 0, sizeof(req));
    int request_provided = 0;

    // Parsing Argomenti
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(server_ip, argv[i + 1], CITY_LEN - 1);
            server_ip[CITY_LEN - 1] = '\0';
            i++;
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            char* input_str = argv[i + 1];
            req.type = input_str[0];
            char* space_ptr = strchr(input_str, ' ');

            if (space_ptr != NULL) {
                strncpy(req.city, space_ptr + 1, CITY_LEN - 1);
            } else {
                if (strlen(input_str) > 1) {
                    strncpy(req.city, input_str + 1, CITY_LEN - 1);
                }
            }
            req.city[CITY_LEN - 1] = '\0';
            request_provided = 1;
            i++;
        }
    }

    if (!request_provided) {
        printf("Errore: Parametro -r obbligatorio.\nUso: %s -s <IP> -p <PORTA> -r \"<t|h|w|p> <citta>\"\n", argv[0]);
        return -1;
    }

    // Inizializzazione Winsock (Necessario solo su Windows)
    #if defined _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            printf("Errore: Fallimento di WSAStartup.\n");
            return -1;
        }
    #endif

    // Creazione Socket
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Errore creazione socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connessione
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore: Impossibile connettersi al server %s:%d.\n", server_ip, port);
        close(client_socket); // 'close' è mappato a 'closesocket' su Windows grazie a protocol.h
        #if defined _WIN32
            WSACleanup();
        #endif
        return -1;
    }
    printf("Connessione stabilita con %s:%d\n", server_ip, port);

    // Invio Richiesta
    if (send(client_socket, (char*)&req, sizeof(req), 0) != sizeof(req)) {
        printf("Errore nell'invio della richiesta.\n");
        close(client_socket);
        return -1;
    }

    // Ricezione Risposta
    weather_response_t resp;
    int bytes_recv = recv(client_socket, (char*)&resp, sizeof(resp), 0);

    if (bytes_recv <= 0) {
        printf("Errore in ricezione o connessione chiusa dal server.\n");
        close(client_socket);
        return -1;
    }

    // Stampa Risultati
    printf("Risposta dal server (%s):\n", server_ip);

    if (resp.status == 1) {
        printf(" -> Citta '%s' non disponibile.\n", req.city);
    } else if (resp.status == 2) {
        printf(" -> Tipo di richiesta '%c' non valido.\n", req.type);
    } else if (resp.status == 0) {
        printf(" -> %s: ", req.city);
        switch (resp.type) {
            case 't': printf("Temperatura: %.1f C\n", resp.value); break;
            case 'h': printf("Umidita: %.1f%%\n", resp.value); break;
            case 'w': printf("Vento: %.1f km/h\n", resp.value); break;
            case 'p': printf("Pressione: %.1f hPa\n", resp.value); break;
            default:  printf("Dato sconosciuto: %.1f\n", resp.value); break;
        }
    } else {
        printf("Ricevuto status sconosciuto (%d).\n", resp.status);
    }

    close(client_socket);
    #if defined _WIN32
        WSACleanup();
    #endif
    return 0;
}
