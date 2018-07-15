Repository del modulo PycoTCP
========================

Informazioni sull'Internet of Threads
------------
PycoTCP si basa sulle due principali librerie che implementano il concetto "Internet of Threads", o anche IoT&T (Internet of Threads and Things)

Il suo scopo e' quello di semplificare l'utilizzo di queste librerire in modo che chiunque possa cominciare a sperimentare questo nuovo approccio alle reti. Per far questo, PycoTCP fornisce uno strumento che permette l'esecuzione di un qualsiasi script Python già esistente (che utilizza il modulo "socket" standard del Python) come se fosse un processo del IoT&T, con il suo indirizzo e collegato ad un VDE (Virtual Distributed Ethernet).

Ha molte applicazioni possibili, ad esempio:
   * Eseguire più server sulla stessa macchina e sulla stessa porta, utilizzando diversi indirizzi IP
   * Creare una rete di processi (ad esempio simulando una intera rete IoT con più processi che si mostrano e si comportano come nodi IoT)
   * Eseguire più server sullo stesso host che interagiscono tra di loro come se fossero su una rete reale

In particolare, usando PycoTCP:
   * Esegui script Python esistenti che usano il modulo "socket" con il loro indirizzo IP
   * Usa librerie WSGI per Python per creare semplici Web Server sull'Internet of Threads

Installazione
------------

### Installa tutte le dipendenze ###

#### Testato su Debian 8.5.0: ####
   * git (Solo per fare il clone del repository)
   * make
   * gcc
   * vde2
   * libvdeplug-dev
   * python-pip
   * python-dev
   * libffi-dev

Poi usa pip per installare questi moduli:
   * setuptools

Ex.:

```bash
sudo apt-get install git make gcc vde2 libvdeplug-dev python-pip python-dev libffi-dev

sudo pip install setuptools
```

### Installa le dipendenze opzionali ###

#### Per usare FDPicoTCP ####

Prima installa FDPicoTCP, poi installa o reinstalla PycoTCP

Per installarlo, visita [il repository git di FDPicoTCP](https://github.com/exmorse/fd_picotcp)

#### Per usare LWIPv6 ####

Usa apt-get per installarlo (su Debian):
   * liblwipv6-dev
Poi installa o reinstalla PycoTCP

Per più informazioni su LWIPv6, visita [La Wiki di VDE, Virtual Square](http://wiki.v2.cs.unibo.it/)

### Dal terminale ###

Clona la repository:

```bash
git clone --recursive https://github.com/LuigiPower/pycotcp.git
cd pycotcp
```

#### Installa per l'utente corrente: ####

```bash
make install
```

##### NOTA #####
Se installi per l'utente corrente, gli eseguibili sono installati in ~/.local/bin che, di default, non è nella variable d'ambiente PATH.

#### Installa per tutti gli utenti: ####

```bash
make globalinstall #richiesti i permessi di root
```

Se tutte le dipendenze sono state installate correttamente, il setup dovrebbe compilare l'ultima versione di PicoTCP, poi installare PycoTCP stesso.
(NOTA: Questo verrà cambiato in una futura patch, quando PicoTCP verrà spostato dalle dipendenze necessarie a quelle opzionali usandi CFFI invece del suo codice sorgente)

Installazione - Risoluzione dei problemi
------------------------------

Per errori durante l'installazione, prima controlla se l'errore è stato causato da PicoTCP, FDPicoTCP, LWIPv6, VDE o PycoTCP stesso.

Per errori di PicoTCP fai riferimento al [repository git di PicoTCP](https://github.com/tass-belgium/picotcp)

Per errori di FDPicoTCP fai riferimento al [repository git di FDPicoTCP](https://github.com/exmorse/fd_picotcp)

Per errori relativi a LWIPv6 o VDE fai riferimento alla [wiki di VDE, Virtual Square](http://wiki.v2.cs.unibo.it/)

Se è un problema relativo a PycoTCP, assicurati di aver installato tutte le dipendenze sul sistema. Se il problema non si risolve, [Contatta lo sviluppatore](mailto:federico.giuggioloni@gmail.com) e proverà a riparare il tuo problema, magari in un aggiornamento della libreria.

Disinstallazione
---------
Se PycoTCP è stato installato come utente:

```bash
pip uninstall pycotcp
```

Altrimenti, esegui lo stesso comando con i permessi di root:

```bash
sudo pip uninstall pycotcp
```

NOTA: Questo serve anche per rimuovere vecchie versioni.

Utilizzo
-----

### Esplicitamente ###

In un qualsiasi script Python:

```python
from pycotcp.picotcpadapter import PicoTCPAdapter
from pycotcp.fdpicotcpadapter import FDPicoTCPAdapter
from pycotcp.lwipv6adapter import Lwipv6Adapter

from pycotcp.pycotcp import Pyco
from pycotcp.pycotcp import DeviceInfo
import pycotcp.socket as socket

#One context type
context = PicoTCPAdapter()
#context = FDPicoTCPAdapter()
#context = Lwipv6Adapter()

ioth = Pyco(context=context)
device = DeviceInfo(context=context).with_type(devtype) \
        .with_name(device) \
        .with_path(filename) \
        .with_address(address) \
        .with_netmask(netmask) \
        .create()

ioth.start_handler() #Only required for PicoTCPAdapter

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, context=context)
sock.bind(("10.40.0.55", "5500"))
#Code using standard socket API
...
...
...

ioth.stop_handler() #Only required for PicoTCPAdapter
```
### Implicitamente, usando pyco-wrapper ###

Scrivi un normale script Python che utilizza i socket:

```python
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.bind(("0.0.0.0", 8090))
s.listen(3)

while True:
    try:
        connection, address = s.accept()
        buf = connection.recv(64)
        if len(buf) > 0:
            print buf
            break
    except:
        print "exception"
        break

s.close()
```

Se la VDE non è stata gia impostata, esegui pyco-wrapper con i permessi di amministratore e il flag -c:

```bash
pyco-wrapper <python-script> -a <address> -d <device_name> -f <vde_switch path> -l <library_name> -c # -c Per creare la VDE (Richiede i permessi di amministratore)
#Library name può essere: picotcp, fdpicotcp, lwipv6 [il default è picotcp]

#Example:
pyco-wrapper script.py -a 10.40.0.32 -d web0 -f /tmp/web0.ctl -l picotcp -c
```

O imposta una VDE manualmente usando pyco-vde-start (installato con PycoTCP):

```bash
pyco-vde-setup start <address/maskbits> <vde_switch control file to create>

#Example:
pyco-vde-setup start 10.40.0.1/24 /tmp/vde.ctl
```

Per disattivare una VDE esistente, usa vdeterm (questo sarà semplificato in una futura patch con il comando 'pyco-vde-setup stop'):

```bash
vdeterm <path to .mgmt file (same path as the .ctl file)>

#Example:
>vdeterm /tmp/vde.mgmt
VDE switch V.2.3.2
(C) Virtual Square Team (coord. R. Davoli) 2005,2006,2007 - GPLv2
> shutdown
```

If a vde_switch already exists, run pyco-wrapper without -c and with the existing switch:
Se un vde_switch esiste già, esegui pyco-wrapper senza -c e con la path dello switch:

```bash
#Si aspetta che un vde_switch esista in /tmp/vde.ctl
pyco-wrapper script.py -a 10.40.0.32 -d web0 -f /tmp/vde.ctl -l picotcp
```

### Per script gia esistenti che utilizzano i Socket ###

Esegui questo comando (usando -c solo se la VDE non è gia stata impostata):

```bash
pyco-wrapper script.py -a <address> -d <dev_name> -f <dev_file_path> -l <library_name> -c #Where library name is picotcp, fdpicotcp or lwipv6 as usual
```

```bash
pyco-wrapper -h #Per vedere tutti i parametri disponibili con una spiegazione
```

