---
title: Comunicazione HTTP da C++ con RayLib
subtitle: Net Wrapper - Guida Pratica per Studenti
author: Alessandro Bugatti
date: 2026
---

# Comunicazione HTTP da C++ con RayLib

Guida didattica all'uso del Net Wrapper per comunicazione client-server

---

# Parte 1: HTTP ad Alto Livello

## Cos'è HTTP?

HTTP = **HyperText Transfer Protocol**

È il "linguaggio" che i computer usano per comunicare su Internet:

- Il **client** (il tuo programma C++) invia una **richiesta** al server
- Il **server** riceve la richiesta, la elabora, e invia una **risposta**
- Il client riceve la risposta e la usa

Esempio quotidiano: quando apri un sito web nel browser

---

## Richiesta HTTP - Esempio

Il tuo programma chiede al server il valore della chiave `"player1pos"`:

```
GET /get/player1pos HTTP/1.1
Host: localhost:3000
```

È come dire al server: *"Ehi, dammi il valore associato a player1pos!"*

---

## Risposta HTTP - Esempio

Il server risponde:

```
HTTP/1.1 200 OK
Content-Type: text/plain

150,200
```

Il `200` significa: "Va bene, ecco quello che mi hai chiesto!"

Se non trova `player1pos`, risponderebbe con un valore vuoto.

---

## Due Tipi di Richiesta

### GET - Recuperare dati

Chiedi al server di darti un valore:

```
GET /get/chiave
```

### POST - Inviare dati

Chiedi al server di salvare un valore:

```
POST /set/chiave
Body: {"value": "nuovo_valore"}
```

---

## Bloccante vs Non-Bloccante

Se il client **aspetta** la risposta del server (bloccante):

```
Richiesta -----> Server -----> Risposta
Tutto fermo     elabora      Posso continuare
```

Se il client **non aspetta** (non-bloccante, **asincrona**):

```
Richiesta -----> Server    <- il mio programma continua
                 elabora     mentre il server lavora
                   |
                   +----> Risposta (arriva dopo)
```

**Il Net Wrapper usa la modalità asincrona** = il tuo programma non si ferma mai!

---

# Parte 2: Architettura del Progetto

## Componenti

```
┌─────────────────┐
│  Tuo programma  │
│  (RayLib)       │
└────────┬────────┘
         │ net_wrapper.hpp
         │ (Client asincrono)
         │
   ┌─────▼─────┐
   │   SERVER  │
   │ Docker @  │
   │ :3000     │
   └───────────┘
```

- **Net Wrapper**: nasconde i dettagli di HTTP
- **Server**: salva e ritorna coppie chiave-valore

---

## Net Wrapper: Cosa Fa

Il wrapper fornisce 5 funzioni semplici:

1. **`init(url)`** - Connetti al server
2. **`ping()`** - Verifica se il server risponde
3. **`get(chiave)`** - Richiedi un valore
4. **`set(chiave, valore)`** - Salva un valore
5. **`pollResponse()`** - Leggi la risposta (nel main loop)

Le richieste **NON bloccano** il tuo programma!

---

## Flusso di un'Operazione Asincrona

```
Frame 1: net::get("player1pos")
         ↓ (entra in coda, funzione ritorna subito)
         
Frame 2-N: Il thread di networking elabora in background
           Il tuo programma continua normalmente
           
Frame M:   if (net::pollResponse(r)) {
             // Risposta arrivata! Usala
           }
```

---

# Parte 3: Come Usare Net Wrapper

## Step 1: Inizializzazione

All'inizio del tuo `main()`:

```cpp
#include "net_wrapper.hpp"

int main() {
    InitWindow(800, 450, "Il mio programma");
    
    net::init("http://localhost:3000");
    
    if (!net::ping()) {
        std::cout << "Errore: server non raggiungibile!\n";
        return 1;
    }
    
    // ... resto del programma
}
```

---

## Step 2: Inviare Richieste (fuori dal loop grafico)

```cpp
// Chiedi un valore al server
net::get("chiave_importante");

// Oppure salva un valore
net::set("player_name", "Mario");

// Queste funzioni ritornano SUBITO
// Non aspettano il server!
```

---

## Step 3: Nel Main Loop - Leggere Risposte

```cpp
SetTargetFPS(60);

while (!WindowShouldClose()) {
    // Controlla se c'è una risposta disponibile
    net::Response r;
    if (net::pollResponse(r)) {
        // Una risposta è arrivata!
        if (r.value.has_value()) {
            std::string valore = r.value.value();
            std::cout << "Ricevuto: " << valore << "\n";
        } else {
            std::cout << "Errore nella richiesta\n";
        }
    }
    
    BeginDrawing();
    // ... disegna
    EndDrawing();
}
```

---

## Cosa Significa `has_value()`?

La risposta potrebbe contenere un valore oppure niente (errore):

```cpp
if (r.value.has_value()) {
    // Il server ha ritornato un valore
    std::string risultato = r.value.value();
} else {
    // Il server ha avuto un errore
    // (chiave non trovata, problema di rete, ecc.)
}
```

È come aprire una scatola: potrebbe contenere qualcosa o essere vuota!

---

# Parte 4: Avvio del Server Docker

## Prerequisito

Devi avere **Docker** installato sulla tua macchina.

[Scarica da: https://www.docker.com/products/docker-desktop](https://www.docker.com)

---

## Avviare il Server

Apri un terminale nella root del progetto:

```bash
cd server-docker
docker compose up -d --build
```

Il server parte e rimane in background sulla porta `3000`.

---

## Verificare che il Server Funzioni

```bash
docker compose ps
```

Dovresti vedere il container `kv-server` in stato `Up`.

---

## Fermare il Server

Quando non ne hai più bisogno:

```bash
cd server-docker
docker compose down
```

---

# Parte 5: Esempio Pratico Completo

## Programma Minimo - 1

Salva una posizione del player, poi la recupera:

```cpp
#include "raylib.h"
#include "net_wrapper.hpp"
#include <iostream>

int main() {
    InitWindow(800, 450, "Player Position Saver");
    
    net::init("http://localhost:3000");
    
    if (!net::ping()) {
        std::cout << "Server non raggiungibile!\n";
        CloseWindow();
        return 1;
    }
```

## Programma Minimo - 2

...segue

```cpp
    // Salva la posizione del player
    net::set("player_x", "250");
    net::set("player_y", "150");
    
    std::string displayText = "In attesa di conferma...";
    int responsesReceived = 0;
    
    SetTargetFPS(60);
```
## Programma minimo - 3

...segue
```cpp
    while (!WindowShouldClose()) {
        // Leggi le risposte
        net::Response r;
        if (net::pollResponse(r)) {
            responsesReceived++;
            if (r.value.has_value()) {
                displayText = "OK: " + r.key + " salvato!";
            } else {
                displayText = "Errore su: " + r.key;
            }
        }
        
```

## Programma minimo - 4

...segue

```cpp
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("Player Position Saver", 30, 20, 20, DARKGRAY);
        DrawText(displayText.c_str(), 30, 100, 20, BLACK);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

---

## Cosa Fa Questo Programma?

1. Connette al server Docker su `localhost:3000`
2. Verifica con `ping()` che il server sia raggiungibile
3. Invia **due richieste** non-bloccanti (set x e y)
4. Nel main loop, aspetta le risposte
5. Mostra sullo schermo il risultato

---

## Sequenza Temporale

```
Frame 1:   init() -> OK
           ping() -> OK
           set("player_x", "250") -> in coda
           set("player_y", "150") -> in coda

Frame 2-5: pollResponse() -> niente ancora
           Il thread di background lavora...

Frame 6:   pollResponse() -> arriva risposta 1
           displayText = "OK: player_x salvato!"

Frame 7:   pollResponse() -> arriva risposta 2
           displayText = "OK: player_y salvato!"

Frame 8+:  pollResponse() -> coda vuota, continua a girare
```

---

## Esercizio 1: Leggi un Valore

Modifica il programma per leggere il valore salvato:

```cpp
// Dopo aver salvato i valori, leggi un valore
net::get("player_x");

// Nel pollResponse(), controlla il risultato
if (r.key == "player_x" && r.value.has_value()) {
    std::string x = r.value.value();
    DrawText(("X: " + x).c_str(), 30, 150, 20, BLACK);
}
```

---

## Esercizio 2: Aggiungi un Tasto

Premi un tasto per inviare una nuova posizione:

```cpp
if (IsKeyPressed(KEY_S)) {
    net::set("player_x", "400");
    net::set("player_y", "300");
    displayText = "Nuovo valore inviato...";
}
```

---

## Note Importanti

1. **Non usare subito i dati**: Dopo `set()` o `get()`, non hai il risultato. Devi aspettare `pollResponse()`.

2. **Timeout**: Se il server non risponde in ~300ms, `ping()` lo sa. Usa timeout più lunghi se serve:
   ```cpp
   if (!net::ping(1000)) {  // attendi fino a 1 secondo
   ```

3. **Errori di rete**: Se il server va giù, `pollResponse()` ritornerà `nullopt` (niente).

---

# Parte 6: Domande Frequenti

## Il programma si blocca se il server non c'è?

**No!** Il wrapper usa un thread separato. Il tuo programma continua a girare.

Se il server non risponde:
- `ping()` ritorna `false`
- `pollResponse()` ritornerà risposte con `nullopt`

---

## Posso fare più richieste contemporaneamente?

**Sì!** Il wrapper ha una coda di richieste:

```cpp
for (int i = 0; i < 10; i++) {
    net::set("key" + std::to_string(i), "valore");
}
// Tutte e 10 le richieste vengono messe in coda
// Il server le elabora una alla volta
```

---

## Come faccio a sapere quale risposta è arrivata?

Usa il campo `r.key`:

```cpp
net::Response r;
if (net::pollResponse(r)) {
    if (r.key == "player_x") {
        // è la risposta per player_x
    } else if (r.key == "player_y") {
        // è la risposta per player_y
    }
}
```

---

## Il server salva i dati davvero?

**Sì!** Usa Docker Compose con un **volume persistente**.

I dati vengono salvati in `server-docker` e rimangono anche se riavvii il container.

Se vuoi azzarare tutto: `docker compose down -v` (rimuove i volumi).

---

# Conclusione

## Quello che hai imparato

✓ HTTP è il "linguaggio" di comunicazione client-server  
✓ Il Net Wrapper nasconde i dettagli di HTTP  
✓ Le operazioni sono **asincrone** = il programma non si blocca  
✓ Usi `init()`, `ping()`, `get()`, `set()`, `pollResponse()`  
✓ Il server gira in Docker e salva i dati  

---

## Prossimi Passi

1. Scarica e installa Docker
2. Avvia il server: `docker compose up -d --build` in `server-docker`
3. Compila il tuo programma
4. Connettiti con `net::init("http://localhost:3000")`
5. Divertiti a creare programmi che salvano e leggono dati!

---

## Domande?

Ricorda:

- Il wrapper fa il lavoro sporco di HTTP per te
- Tu usi solo 5 funzioni semplici
- Le richieste sono veloci e non bloccano
- Il server è sempre disponibile su localhost:3000

**Buon coding!** 🚀

