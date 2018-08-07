
pyvdetelweb
========================

pyvdetelweb è un tool per la gestione di reti VDE da remoto, scritto in Python utilizzando la libreria PyctoTCP.
Offre il collegamento tramite interfaccia telenet o server web.

Per avviarlo:
```
pyvdetelweb -web -telnet -s /tmp/vde.ctl -m /tmp/vde.mgmt
```
Dove __-s__ indica la posizione della directory di controllo dello switch vde e  __-m__ è il manamgment socket di un switch vde

Configurazione
---------------
Il file di configuraione 'vdetelweb.rc' è installato di default in '/etc/vde/pyvdetelweb' e contiene i paramentri necessari per l'avvio del tool:

```
#vdetelweb rc sample
ip=192.168.250.5
netmask=255.255.255.0
# to create a new passwd run the following command:
# $ echo -n piripicchio | sha1sum
# typing your passwd instead of "piripicchio"
username=admin
password=e8b32ad31b34a21d9fa638c2ee6cf52d46d5106b
stack=picotcp
```

con __stack=__ è possibile specificare il tipo di stack di rete che il processo andrà ad utilizzare (deafult:[picotcp](https://github.com/tass-belgium/picotcp)).
Stack supportati da PycoTCP:
```
...
'stack=lwipv6' -> LWIPV6 (http://wiki.v2.cs.unibo.it/wiki/index.php/LWIPV6)
'stack=picotcp' -> PicoTCP (https://github.com/tass-belgium/picotcp)
'stack=fdpicotcp' -> fd_picoTCP (https://github.com/exmorse/fd_picotcp)
```

Installazione
-------------

```
git clone

cd ./pyvdetelweb

pip install .
```
### Dipendenze ###

La libreria PycoTCP è necessaria per connettersi alle reti VDE
Segui le istruzioni in *pyvdetelweb/pycotcp/README-it.md* per installare il modulo e le sue dipendenze.

Per più informazioni su VDE, visita [La Wiki di VDE, Virtual Square](http://wiki.v2.cs.unibo.it/)

REST API
--------
Pyvdetelweb grantisce anche un interfaccia tramite __web API__ (architettura [RESTful](https://en.wikipedia.org/wiki/Representational_state_transfer)) bastato su protocollo HTTP con cui è possibile comunicare con il __mangment sokcet__ dello switch VDE.
Questo servizio viene utilizzato anche dall'interfaccia HTML per inviare richieste al server tramite chiamate asincrone AJAX.

Le chiamate sono differenziate tramite metodo GET (per avere informazioni relative al comando) e POST (per eseguire il comando).
Le route sono disposte gerarchicamente ad albero rispettando la struttura dei comandi da terimnale, e avendo come prefisso __/api__ (es. '/api/port/print').

Il server risponde con un JSON riportando il dettagli della richiesta e il contentuo della della riposta in maniera così strutturata:

- GET:
```
{
 commands : "lista di commandi raggiungibile dalla path specificata"
 resource : "path del comando richiesto"
 showinfo : "informazioni relative al manamgment socket"
 terminal_prefix : "prefisso del terminale con informazoni relative alla connesione"
}
```

- POST:
```
{
  command : comando inviato inviato al terminale
  arguments : argomenti relativi al comando
  response : risposta ristornata dal terminale
  terminal_prefix : prefisso del terminale con informazoni relative alla connesione
}
```

L'accesso alle API richiede autenticazione basta su [HTTTP BASIC AUTHENTICATION](https://en.wikipedia.org/wiki/Basic_access_authentication)
aggiungendo l'header HTTP Authorization alle chiamate:
`Authorization: Basic QWxhZGRpbjpPcGVuU2VzYW1l`
