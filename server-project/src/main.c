#include "protocol.h"
#include <time.h>

// --- FIX PER WINDOWS ---
#if defined _WIN32
    #define strcasecmp _stricmp
#endif

// --- FUNZIONI DI UTILITÀ (Spostate SOPRA il main per evitare errori) ---

float get_random_float(float min, float max) {
    return min + (rand() / (float) RAND_MAX) * (max - min);
}

// Implementazione funzioni meteo
float get_temperature() { return get_random_float(-10.0, 40.0); }
float get_humidity()    { return get_random_float(20.0, 100.0); }
float get_wind()        { return get_random_float(0.0, 100.0); }
float get_pressure()    { return get_random_float(950.0, 1050.0); }

// Verifica città valida
int is_city_valid(const char* city) {
    const char* valid_cities[] = {
        "bari", "roma", "milano", "napoli", "torino",
        "palermo", "genova", "bologna", "firenze", "venezia "
    };
    for (int i = 0; i < 10; i++) {
        if (strcasecmp(city, valid_cities[i]) == 0) return 1; // Trovata
    }
    return 0; // Non trovata
}

// --- MAIN ---
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    srand((unsigned int)time(NULL));

    // 1. Lettura Porta
    int port = DEFAULT_PORT;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) port = atoi(argv[2]);
    printf("Server avviato sulla porta %d\n", port);

    // 2. Setup Windows
    #if defined _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) return -1;
    #endif

    // 3. Setup Socket (Create -> Bind -> Listen)
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_IP);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore Bind.\n");
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) < 0) {
        printf("Errore Listen.\n");
        return -1;
    }
    printf("In attesa di connessioni...\n");

    // 4. Ciclo Infinito
    // ... dentro il main del Server ...

        // 4. Ciclo Infinito
        while (1) {
            struct sockaddr_in client_addr;
            int client_len = sizeof(client_addr);

            // Accetta connessione
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket < 0) continue;

            // Ricevi richiesta
            weather_request_t req;
            // Nota: recv deve leggere esattamente sizeof(req)
            int bytes_read = recv(client_socket, (char*)&req, sizeof(req), 0);

            if (bytes_read != sizeof(req)) {
                close(client_socket);
                continue;
            }

            req.city[63] = '\0'; // Sicurezza stringa

            // --- CORREZIONE CRITICA QUI SOTTO ---
            // Il testo chiede: Richiesta '<type> <city>' dal client ip <ip_address>
            printf("Richiesta '%c %s' dal client ip %s\n",
                   req.type, req.city, inet_ntoa(client_addr.sin_addr));
            // ------------------------------------

            // Prepara risposta
            weather_response_t resp = {0};
            resp.type = req.type;

            // Logica decisionale
            if (!is_city_valid(req.city)) {
                resp.status = 1; // Città errata
                // NOTA: Il prof chiede value=0.0 e type='\0' in caso di errore
                resp.value = 0.0;
                if (resp.status != 0) resp.type = '\0'; // Come da specifica "Se non valida... Imposta type = '\0'"
            } else {
                // Verifica validità Tipo
                 int type_ok = (req.type == 't' || req.type == 'h' || req.type == 'w' || req.type == 'p');

                 if (!type_ok) {
                     resp.status = 2;
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

    return 0;
}
// da aggiungere in visualizzazione tutte le fasi di connessione e creazione del socket
//quindi avere a vista ack syn etc
// rendere realistiche le temperature
// vedere
