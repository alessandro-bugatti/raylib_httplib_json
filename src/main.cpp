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
