#include "raylib.h"
#include "net_wrapper.hpp"
#include <string>

int main() {
    InitWindow(800, 450, "RayLib + cpp-httplib demo");

    // Server di test (es. un tuo microservizio)
    net::init("http://localhost:3000");

    std::string lastValue;
    if (net::ping()) {
        // invia una GET solo se il server risponde
        net::get("test");
        lastValue = "In attesa di risposta...";
    } else {
        lastValue = "Server non raggiungibile su localhost:3000";
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // controlla se ci sono risposte dal thread
        net::Response r;
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
