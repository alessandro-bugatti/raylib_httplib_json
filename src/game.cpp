#include "raylib.h"
#include "net_wrapper.hpp"
#include <string>
#include <algorithm>

struct Comandi
{
    int up, down, left, right;
};

int main() {

    InitWindow(800, 450, "RayLib + cpp-httplib demo");

    // Server di test (es. un tuo microservizio)
    net::init("http://localhost:3000");

    net::ping();

    int giocatore = 0;
    int x = GetRandomValue(0, 780), y = GetRandomValue(0, 430);
    int xn = 0, yn = 0;
    net::Response r;
    Comandi comandi{};
    Color colori[] = {BLUE, RED};

    //Recupera l'id del giocatore, che può essere 0, per il primo giocatore che si
    //connette o 1 per il secondo
    net::get("giocatore");
    while (!net::pollResponse(r)) {}
    if (r.key == "giocatore" && r.value.has_value()) {
        //Se per qualunque motivo la chiave non fosse un numvero valido,
        //verrà fatto come se fosse 0
        try {
            giocatore = stoi(r.value.value());
        } catch (...) {
            giocatore = 0;
        }
        if (giocatore < 0 || giocatore > 1) {
            giocatore = 0;
        }
    }
    net::set("giocatore", std::to_string(giocatore + 1));

    //Mette comandi diversi, utile quando si vuole provare in locale,
    //per non confondersi. IL primo che si connette avrà WASD, il secondo le frecce
    if (giocatore == 1) {
        comandi.up = KEY_UP;
        comandi.down = KEY_DOWN;
        comandi.left = KEY_LEFT;
        comandi.right = KEY_RIGHT;
    }
    else {
        comandi.up = KEY_W;
        comandi.down = KEY_S;
        comandi.left = KEY_A;
        comandi.right = KEY_D;
    }

    const std::string mio_giocatore = "g" + std::to_string(giocatore);
    const std::string avversario = "g" + std::to_string(1 - giocatore);

    //Il primo giocatore che si connette imposta le posizioni iniziali casuali e
    //le scrive sul server. Le posizioni sono nelle chiavi g0 e g1,
    //sotto forma di stringa x-y, con x e y numeri interi
    if (giocatore == 0) {
        int x0 = GetRandomValue(0, 780);
        int y0 = GetRandomValue(0, 430);
        int x1 = GetRandomValue(0, 780);
        int y1 = GetRandomValue(0, 430);

        x = x0;
        y = y0;
        xn = x1;
        yn = y1;

        net::set("g0", std::to_string(x0) + "-" + std::to_string(y0));
        net::set("g1", std::to_string(x1) + "-" + std::to_string(y1));
    }

    //Intervallo di aggiornamento delle informazioni prese dal server
    const double networkIntervalSec = 1.0 / 30.0;
    double nextNetworkTick = GetTime();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        //Aggiorna la posizione del giocatore
        if (IsKeyDown(comandi.up)) {
            y--;
        }
        if (IsKeyDown(comandi.down)) {
            y++;
        }
        if (IsKeyDown(comandi.left)) {
            x--;
        }
        if (IsKeyDown(comandi.right)) {
            x++;
        }

        x = std::clamp(x, 0, 780);
        y = std::clamp(y, 0, 430);

        const double now = GetTime();
        //Con la frequenza settata in nextNetworkTick scrive la propria
        //posizione sul server e legge quella dell'avversario
        if (now >= nextNetworkTick) {
            //Scrive la propria posizione
            net::set(mio_giocatore, std::to_string(x) + "-" + std::to_string(y));
            //Legge quella dell'aversario
            net::get(avversario);
            nextNetworkTick = now + networkIntervalSec;
        }

        //Controlla i valori presenti sul server
        while (net::pollResponse(r)) {
            //Se è il proprio valore viene ignorato
            if (!r.value.has_value() || r.key == mio_giocatore) {
                continue;
            }
            //Verifica se è un valore di posizione perchè contiene il carattere -
            const std::string &value = r.value.value();
            size_t sep = value.find('-');
            //Se non è un valore di posizione viene ignorato
            if (sep == std::string::npos) {
                continue;
            }
            //Legge la posizione dell'avversario e la aggiorna nel gioco locale
            try {
                xn = stoi(value.substr(0, sep));
                yn = stoi(value.substr(sep + 1));
                xn = std::clamp(xn, 0, 780);
                yn = std::clamp(yn, 0, 430);
            } catch (...) {
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        //Disegna il giocatore
        DrawRectangle(x, y, 20, 20, colori[giocatore]);
        //Disegna l'avversario
        DrawRectangle(xn, yn, 20, 20, colori[1 - giocatore]);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
