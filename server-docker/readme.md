## Come testare il server

Per testare se il server funziona, nella cartella `server-docker` si trova il file
`rest-api-test.http` che, in un formato gestito da CLion, permette di eseguire delle
prove delle seguenti funzionalità riassunte qua sotto. 
Per eseguirle premere sulla freccia verde in corrispondenza delle funzionalità che
si vuole testare, che compare all'apertura del file `rest-api-test.http` presente
in questa cartella.


1. GET — Ping
```shell
http# @timeout 500
GET http://localhost:3000/get/ping
```
Verifica che il server sia raggiungibile usando la chiave riservata **ping** (andrebbe bene qualsiasi altra). Il commento `# @timeout 500` imposta un timeout di 500 ms: se il server non risponde entro quel limite, la richiesta fallisce. Utile per health check rapidi.

2. GET — Recupera un valore
```shell
http GET http://localhost:3000/get/chiave
```
Recupera il valore associato a una chiave. Sostituire chiave nel path con la chiave desiderata (es. get/username).

3. POST — Imposta un valore
```shell
http POST http://localhost:3000/set/chiave
Content-Type: application/json
{
"value": "il_tuo_valore"
}
```

Imposta un valore per la chiave specificata. Sostituire chiave nel path con il nome della chiave e "il_tuo_valore" con 
il valore da memorizzare. Le chiavi, una volta impostate, sopravvivono al riavvio del server. Se le si volesse 
cancellare seguire le istruzioni presenti nel `readme` principale.

