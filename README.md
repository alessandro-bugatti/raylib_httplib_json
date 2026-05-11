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
3. Invia richieste non bloccanti con `net::set(...)` e/o `net::get(...)`.
4. Nel main loop, chiama `net::pollResponse(...)` a ogni frame per leggere le risposte.

Esempio completo minimale:

```cpp
#include "net_wrapper.hpp"
#include <iostream>

int main() {
    net::init("http://localhost:3000");

    if (!net::ping()) {
        std::cout << "Server non raggiungibile\n";
        return 1;
    }

    net::set("player1pos", "150,200");
    net::get("player1pos");

    // Simula il main loop
    while (true) {
        net::Response r;
        if (net::pollResponse(r)) {
            if (r.value.has_value()) {
                std::cout << "OK " << r.key << " = " << r.value.value() << "\n";
            } else {
                std::cout << "Errore su chiave: " << r.key << "\n";
            }
            break;
        }
    }
}
```

Nota: `set/get` non bloccano il thread principale; la lettura risultato avviene sempre tramite `pollResponse(...)`.

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
