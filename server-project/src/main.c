#include "protocol.h"

// Funzioni Meteo (Simulazione)

float get_random_float(float min, float max) {
    return min + (rand() / (float) RAND_MAX) * (max - min);
}

float get_temperature() { return get_random_float(5.0, 35.0); }
float get_humidity()    { return get_random_float(40.0, 95.0); }
float get_wind()        { return get_random_float(0.0, 50.0); }
float get_pressure()    { return get_random_float(980.0, 1030.0); }


int is_city_valid(const char* city) {
    const char* valid_cities[] = {
        "bari", "roma", "milano", "napoli", "torino",
        "palermo", "genova", "bologna", "firenze", "venezia"
    };
    for (int i = 0; i < 10; i++) {
        if (strcasecmp(city, valid_cities[i]) == 0) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    srand((unsigned int)time(NULL));

    int port = DEFAULT_PORT;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }
    printf("Server in ascolto sulla porta %d\n", port);

    // Setup Winsock (solo Windows)
    #if defined _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
             printf("Errore critico: WSAStartup fallita.\n");
             return -1;
        }
    #endif

    // Creazione Socket
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Impossibile creare il socket.\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Accetta da qualsiasi interfaccia

    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore Bind. La porta %d e' forse occupata?\n", port);
        close(server_socket);
        return -1;
    }

    // Listen
    if (listen(server_socket, SOMAXCONN) < 0) {
        printf("Errore durante la fase di Listen.\n");
        close(server_socket);
        return -1;
    }
    printf("In attesa di connessioni...\n");

    // Ciclo Principale
    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);

        // Accept
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
        if (client_socket < 0) continue;

        // Ricezione richiesta
        weather_request_t req;
        int bytes_read = recv(client_socket, (char*)&req, sizeof(req), 0);

        if (bytes_read != sizeof(req)) {
            close(client_socket);
            continue;
        }

        req.city[CITY_LEN - 1] = '\0';
        printf("Ricevuta richiesta '%c %s' da IP %s\n",
                         req.type, req.city, inet_ntoa(client_addr.sin_addr));

        // Elaborazione
        weather_response_t resp = {0};
        resp.type = req.type;

        if (!is_city_valid(req.city)) {
            resp.status = 1; // Città non trovata
        } else {
            int type_ok = (req.type == 't' || req.type == 'h' || req.type == 'w' || req.type == 'p');

            if (!type_ok) {
                resp.status = 2; // Tipo non valido
            } else {
                resp.status = 0; // OK
                if (req.type == 't') resp.value = get_temperature();
                else if (req.type == 'h') resp.value = get_humidity();
                else if (req.type == 'w') resp.value = get_wind();
                else if (req.type == 'p') resp.value = get_pressure();
            }
        }

        // Invio e chiusura
        send(client_socket, (char*)&resp, sizeof(resp), 0);
        close(client_socket);
    }

    close(server_socket);
    #if defined _WIN32
         WSACleanup();
    #endif
    return 0;
}
