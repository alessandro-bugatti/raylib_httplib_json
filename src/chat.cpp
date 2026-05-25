#include "raylib.h"
#include "net_wrapper.hpp"
#include <string>

int main() {
    InitWindow(800, 450, "Chat connettere il mondo");

    // Server di test (es. un tuo micro-servizio)
    net::init("http://localhost:3000");

    std::string lastValue;

    if (net::ping()) {
        lastValue = "In attesa di risposta...";
    } else {
        lastValue = "Server non raggiungibile su localhost:3000";
    }

    SetTargetFPS(60);

    std::string messaggio;
    net::Response r;
    double istante = GetTime();
    const int AGGIORNAMENTI_PER_SECONDO = 2;
    const double DELTA = 1.0 / AGGIORNAMENTI_PER_SECONDO;
    while (!WindowShouldClose()) {

        char c = GetCharPressed();
        if (c > 0) {
            messaggio += c;
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !messaggio.empty()) {
            messaggio.pop_back();
        }

        // Aggiorna il valore prendendolo dal server
        if (GetTime() - istante > DELTA) {
            net::get("messaggio");
            istante = GetTime();
        }
        // Aggiorna il valore scrivendolo sul server
        if (IsKeyPressed(KEY_ENTER)) {
            // scrive il contatore sul server
            net::set("messaggio", messaggio);
        }
        // se è presente
        if (net::pollResponse(r)) {
            if (r.value.has_value())
                lastValue = "Messaggio sul server: " + r.value.value();
            else
                lastValue = "Errore nella richiesta.";
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText(lastValue.c_str(), 30, 80, 30, BLACK);
        std::string temp = "Messaggio: " + messaggio;
        DrawText(temp.c_str(), 30, 200, 30, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}