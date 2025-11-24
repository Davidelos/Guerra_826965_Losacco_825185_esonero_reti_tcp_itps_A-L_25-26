#include "protocol.h"

// --- FUNZIONI DI UTILITÀ ---

float get_random_float(float min, float max) {
    // Generazione del float casuale
    return min + (rand() / (float) RAND_MAX) * (max - min);
}

// Implementazione funzioni meteo (realistiche solo nell'intervallo)
float get_temperature() { return get_random_float(5.0, 35.0); }
float get_humidity()    { return get_random_float(40.0, 95.0); }
float get_wind()        { return get_random_float(0.0, 50.0); }
float get_pressure()    { return get_random_float(980.0, 1030.0); }

// Verifica città valida
int is_city_valid(const char* city) {
    const char* valid_cities[] = {
        "bari", "roma", "milano", "napoli", "torino",
        "palermo", "genova", "bologna", "firenze", "venezia"
    };
    for (int i = 0; i < 10; i++) {
        // Uso strcasecmp (o _stricmp su Windows) per confronto case-insensitive
        if (strcasecmp(city, valid_cities[i]) == 0) return 1;
    }
    return 0;
}

// --- MAIN DEL SERVER ---
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    srand((unsigned int)time(NULL));

    // 1. Lettura Porta (Opzionale)
    int port = DEFAULT_PORT;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }
    printf("Server avviato sulla porta %d\n", port);

    // 2. Setup Windows (WSADATA)
    #if defined _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            printf("Errore WSAStartup fallita.\n");
            return -1;
        }
    #endif

    // 3. Setup Socket (Create -> Bind -> Listen)
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Errore creazione socket.\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // !!! CORREZIONE CRITICA: Usa INADDR_ANY per accettare su tutte le interfacce !!!
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore Bind. Controlla che la porta %d non sia già in uso.\n", port);
        close(server_socket);
        #if defined _WIN32
             WSACleanup();
        #endif
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) < 0) {
        printf("Errore Listen.\n");
        close(server_socket);
        #if defined _WIN32
             WSACleanup();
        #endif
        return -1;
    }
    printf("In attesa di connessioni...\n");

    // 4. Ciclo Infinito
    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);

        // Accetta connessione
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
        if (client_socket < 0) continue;

        // Ricevi richiesta
        weather_request_t req;
        int bytes_read = recv(client_socket, (char*)&req, sizeof(req), 0);

        if (bytes_read != sizeof(req)) {
            // Ricezione incompleta o fallita
            close(client_socket);
            continue;
        }

        // Assicurati che la stringa sia terminata correttamente (anche se la memset all'inizio dovrebbe farlo)
        req.city[CITY_LEN - 1] = '\0';

        printf("Richiesta '%c %s' dal client ip %s\n",
                    req.type, req.city, inet_ntoa(client_addr.sin_addr));

        // Prepara risposta
        weather_response_t resp = {0};
        resp.type = req.type;

        // Logica decisionale
        if (!is_city_valid(req.city)) {
            resp.status = 1; // Città errata
            resp.value = 0.0;
            resp.type = '\0';
        } else {
            // Verifica validità Tipo
            int type_ok = (req.type == 't' || req.type == 'h' || req.type == 'w' || req.type == 'p');

            if (!type_ok) {
                resp.status = 2; // Tipo non valido
                resp.value = 0.0;
                resp.type = '\0';
            } else {
                resp.status = 0; // OK
                if (req.type == 't') resp.value = get_temperature();
                else if (req.type == 'h') resp.value = get_humidity();
                else if (req.type == 'w') resp.value = get_wind();
                else if (req.type == 'p') resp.value = get_pressure();
            }
        }

        // Invia e Chiudi
        send(client_socket, (char*)&resp, sizeof(resp), 0);
        close(client_socket);
    }

    // Pulizia
    close(server_socket);
    #if defined _WIN32
         WSACleanup();
    #endif
    return 0;
}
