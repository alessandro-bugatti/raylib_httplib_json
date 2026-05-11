// server_simple.js
const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
app.use(express.json());

// Percorso al file persistente (grazie al volume Docker)
const DATA_DIR = "/app/data";
const DB_FILE = path.join(DATA_DIR, "db.json");

// Assicura che /app/data esista
if (!fs.existsSync(DATA_DIR)) {
    fs.mkdirSync(DATA_DIR, { recursive: true });
}

// Carica il DB dal file, o crea un oggetto vuoto
let db = {};
if (fs.existsSync(DB_FILE)) {
    try {
        db = JSON.parse(fs.readFileSync(DB_FILE, "utf8"));
        console.log("✔ Database caricato da db.json");
    } catch (err) {
        console.error("Errore nel leggere db.json, riparto con DB vuoto.");
        db = {};
    }
} else {
    console.log("Nessun db.json trovato, ne creo uno nuovo.");
    fs.writeFileSync(DB_FILE, JSON.stringify(db, null, 2));
}

// Funzione helper per salvare il DB
function saveDB() {
    fs.writeFileSync(DB_FILE, JSON.stringify(db, null, 2));
}

// --- API -------------------------------------------------------------

app.get('/get/:key', (req, res) => {
    const key = req.params.key;
    res.send(db[key] || "");
});

app.post('/set/:key', (req, res) => {
    const key = req.params.key;
    const value = req.body.value;

    db[key] = value;
    saveDB();

    res.send("OK");
});

// --------------------------------------------------------------------

app.listen(3000, () => {
    console.log("🚀 Server KV in ascolto sulla porta 3000");
    console.log("🗂  Persistenza attiva su", DB_FILE);
});

