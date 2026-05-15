# Net Wrapper C++ per comunicazione Client-Server

Questo progetto include un **wrapper C++** (`net_wrapper.hpp`) per interagire con un semplice server key–value HTTP in maniera **asincrona**.  
È pensato per essere usato insieme a **RayLib** o qualsiasi altro main loop non bloccante.

Il wrapper nasconde i dettagli di networking e JSON, e fornisce un’interfaccia semplice: inviare e ricevere dati senza bloccare il programma principale.

Il file `slides.md` contiene le slide di spiegazione sulle idee e l'utilizzo di questo progetto, per produrre la versione HTML si 
può utilizzare il seguente comando
```shell
pandoc slides.md -t revealjs -s -o slides.html --slide-level=2   -V theme=solarized  -V revealjs-url=https://unpkg.com/reveal.js@4   -V width=1280   -V height=720
```

---

## Requisiti

- C++17 o superiore
- [RayLib](https://www.raylib.com/)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) (incluso)
- [nlohmann/json](https://github.com/nlohmann/json) (incluso)
- Docker installato sulla macchina di sviluppo (con Docker Compose)
- Server HTTP REST key-value, presente nel progetto nella cartella _server-docker_, sotto forma di container Docker

---

## Funzioni principali

### 1. `net::init(serverUrl)`

Inizializza il wrapper e avvia un thread di networking che comunica con il server.

```cpp
#include "net_wrapper.hpp"

int main() {
    // Indirizzo del server
    net::init("http://localhost:3000");

    // ... resto del main loop
}
```

### 2. `net::ping(timeoutMs = 300)`

Verifica rapidamente se il server è raggiungibile.

- Restituisce `true` se il server risponde.
- Restituisce `false` se il server non risponde o se `net::init(...)` non è ancora stato chiamato.

```cpp
if (net::ping()) {
    // server raggiungibile
}
```

È possibile impostare un timeout personalizzato (in millisecondi):

```cpp
if (!net::ping(500)) {
    std::cout << "Server non raggiungibile" << "\n";
}
```

### 3. `net::set(key, value)`

Invia una coppia chiave-valore al server. Non blocca il main loop.

```cpp
net::set("player1pos", "150,200");
```

### 4. `net::get(key)`

Richiede al server il valore associato a una chiave. Non blocca il main loop.

```cpp
net::get("player1pos");
```

### 5. `net::pollResponse(response)`

Controlla se ci sono risposte disponibili dal server.

- `response` è di tipo `net::Response`

```cpp
struct Response {
    std::string key;
    std::optional<std::string> value;
};
```

- Restituisce `true` se c’è una risposta, `false` altrimenti.
- Se `response.value` è `std::nullopt`, la richiesta è fallita.

```cpp
net::Response r;
if (net::pollResponse(r) == true) {
    if (r.value.has_value()) {
        std::cout << "Valore ricevuto per " << r.key << ": " << r.value.value() << "\n";
    } else {
        std::cout << "Errore nella richiesta per " << r.key << "\n";
    }
}
```

---

## Flusso consigliato nel main loop

Ordine suggerito per usare il wrapper in modo semplice e robusto:

1. Chiama `net::init("http://localhost:3000")` una sola volta all'avvio.
2. Verifica la raggiungibilita del server con `net::ping()`.
3. Invia richieste non bloccanti con `net::set(...)` e/o `net::get(...)` secondo la logica del programma.
4. Nel main loop, chiama `net::pollResponse(...)` a ogni ciclo per leggere eventuali risposte.

Esempio completo minimale:

```cpp
#include "raylib.h"
#include "net_wrapper.hpp"
#include <string>

int main() {
    InitWindow(800, 450, "RayLib + cpp-httplib demo");

    // Server di test (es. un tuo microservizio)
    net::init("http://localhost:3000");

    std::string lastValue;
    int valore = 0;
    if (net::ping()) {
        lastValue = "In attesa di risposta...";
    } else {
        lastValue = "Server non raggiungibile su localhost:3000";
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // controlla se ci sono risposte dal thread
        net::Response r;
        // Aggiorna il valore prendendolo dal server
        // Con l'if l'aggiornamento avviene solo se richiesto
        // tramite la pressione del tasto R. Se invece si volesse
        // averlo sempre, basterebbe rimuovere l'if
        if (IsKeyPressed(KEY_R)) {
            // chiede l'informazione che interessa
            net::get("test");
        }
        // Aggiorna il valore scrivendolo sul server
        if (IsKeyPressed(KEY_S)) {
            // scrive il contatore sul server
            valore++;
            net::set("test", std::to_string(valore));
        }
        // se è presente
        if (net::pollResponse(r)) {
            if (r.value.has_value())
                lastValue = "Valore ricevuto: " + r.value.value();
            else
                lastValue = "Errore nella richiesta.";
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("RayLib + HTTP async test", 30, 20, 20, DARKGRAY);
        DrawText(lastValue.c_str(), 30, 80, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

Nota: `set/get` non bloccano il thread principale; la lettura del risultato avviene sempre tramite `pollResponse(...)`.

---

## Avvio rapido del server di test

Per provare i programmi C++ che usano `net_wrapper.hpp`, avvia il server dalla cartella `server-docker`.

```bash
cd server-docker
docker compose up -d --build
```

Il server risponde su `http://localhost:3000`.

Per fermarlo:

```bash
cd server-docker
docker compose down
```

Siccome il server mantiene la chiavi memorizzate anche quando viene fermato, se si 
volessero cancellare tutte le chiavi presenti eseguire:

```bash
cd server-docker
docker compose down -v
```