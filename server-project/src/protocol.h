/*
 * protocol.h
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// --- SEZIONE 1: Librerie Cross-Platform ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Aggiunto per l'uso in Server

#if defined _WIN32
    // Se siamo su Windows
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // Definiamo 'close' come 'closesocket' per uniformità
    #define close closesocket
    // Definizione per la funzione case-insensitive (usata nel Server)
    #define strcasecmp _stricmp

    // Inizializzatore necessario per Windows
    void clear_winsock() { WSACleanup(); }
#else
    // Se siamo su Linux o macOS
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

// --- SEZIONE 2: Costanti ---
#define DEFAULT_PORT 56700
// Ho mantenuto l'IP originale, ma useremo 127.0.0.1 per il test locale
#define DEFAULT_IP "172.29.25.122"
#define BUFFER_SIZE 512
#define CITY_LEN 64

// --- SEZIONE 3: Strutture Dati (Come da specifica) ---

// Messaggio di Richiesta (Client -> Server)
typedef struct {
    char type;         // 't', 'h', 'w', 'p'
    char city[CITY_LEN]; // Nome città (terminato da \0)
} weather_request_t;

// Messaggio di Risposta (Server -> Client)
typedef struct {
    unsigned int status; // 0=OK, 1=No Città, 2=Invalid Type
    char type;           // Eco della richiesta
    float value;         // Valore meteo
} weather_response_t;

// --- SEZIONE 4: Prototipi Funzioni Server ---
float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);

#endif /* PROTOCOL_H_ */
