FROM node:18-alpine

# Crea directory app
WORKDIR /app

# Copia solo il package definito al volo
COPY server_simple.js .

# Creiamo un package.json minimale direttamente
RUN echo '{ \
  "name": "simple-kv-server", \
  "version": "1.0.0", \
  "dependencies": { \
    "express": "^4.18.2" \
  } \
}' > package.json

# Installa dipendenze
RUN npm install --production

# Espone la porta
EXPOSE 3000

# Comando di avvio
CMD ["node", "server_simple.js"]

