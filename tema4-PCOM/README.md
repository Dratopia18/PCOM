Tema 4 PCOM - Client web HTTP
de Andrei-Vlad Racolta - 325CD

## Descriere
In aceasta tema, am avut de facut un client web HTTP care sa poata interactiona cu un REST API.
Serverul expune un API, care primeste request-uri de la client si le proceseaza. In timp ce
clientul trimite request-uri catre server, acesta din urma ii trimite inapoi raspunsuri.
In tema, am avut de facut o aplicatie basic, in care sa facem urmatoarele functii:
inregistrarea unui utilizator, autentificarea unui utilizator, accesarea unei librarii de carti,
adaugarea si stergerea unei carti din libraria personala, vizualizarea unei carti sau a tuturor
cartilor din libraria personala, dezautentificarea unui utilizator si o functie de iesire din
aplicatie. Toate aceste functii trebuie sa foloseasca request-uri HTTP de tip GET, POST, si DELETE,
ce sunt implementate in requests.c.

## Fisierul requests.c
Inainte de a ne uita pe tema in sine, trebuie mai intai sa ne uitam la fisierul requests.c.
Acesta include 3 functii principale, care sunt folosite pentru a face request-uri HTTP catre server.

1. `compute_get_request` - Aceasta functie este folosita pentru a face un request de tip GET catre
server. Aceasta primeste ca parametrii un host, un url, un parametru de query, un cookie, numarul
de cookie-uri, si un JWT token. Luam o linie de request, si o completam cu metoda, urlul, protocolul,
parametrul de query (in cazul in care exista), hostul, portul, mesajul "Connection: keep-alive", cookieul si
JWT tokenul. La final, returnam requestul.

2. `compute_post_request` - Aceasta functie este folosita pentru a face un request de tip POST catre
server. Aceasta primeste ca parametrii un host, un url, un tip de continut, un body, un cookie si un JWT token.  
Luam o linie de request, si o completam cu metoda, urlul, protocolul, hostul, portul, tipul de continut, 
lungimea continutului, cookieul, JWT tokenul si bodyul. La final, returnam requestul.

3. `compute_delete_request` - Aceasta functie este folosita pentru a face un request de tip DELETE catre
server. In afara de faptul ca trimitem un request de tip DELETE, aceasta functie este identica cu
`compute_get_request`. 

## Fisierul client.c
Acum ca am vazut cum facem request-uri catre server, putem sa ne uitam la fisierul client.c.
Aici o sa gasim functiile care implementeaza functionalitatile cerute in tema. Inainte de a incepe
prezentarea functiilor, trebuie mentionat urmatoarele lucruri:

a. Definirea unor constante - In tema, am avut nevoie de host, de port si de rutele pentru a face
request-uri catre server. Acestea sunt definite la inceputul fisierului, pentru un cod mai frumos ochiului,
acestea avand nume sugestive, precum `HOST`, `PORT`, `REGISTER`, `LOGIN`, `PAYLOAD`, `ACCESS`, `BOOKS`
si `LOGOUT`.

b. Parsarea informatiilor in JSON - In tema, pentru a trimite informatii catre server, acestea trebuie
pasate sub forma de JSON. Pentru a face asta, am descarcat de pe Github Parson, o librarie in C, special
creata pentru a face parsing de JSON. Aceasta librarie este inclusa in fisierul client.c, si este folosita
in functiile care fac request-uri de tip POST, pentru a trimite JSON-ul intr-un format corect. Aveam alternativa
de a face parsing de JSON manual, dar am ales sa folosesc aceasta librarie pentru a face codul mai usor de
citit si de inteles, plus ca mi-a fost mai usor sa o folosesc decat sa fac parsing manual.

c. De asemenea, e important de mentionat ca cookieul, JWT tokenul si ultimul ID sters sunt
retinute in variabile globale, pentru a putea fi folosite in toate functiile ce au nevoie de ele.

Acum ca am mentionat aceste lucruri, putem sa ne uitam la functiile in sine.

1. `register_user` - Aceasta functie este folosita pentru a inregistra un utilizator. Declaram message, response si
sockfd. Verificam daca cookieul e deja setat, daca da, afisam un mesaj de eroare si iesim din functie. Daca nu, 
deschidem conexiunea, luam username-ul si parola de la utilizator, verificam daca acestea au
caractere invalide, daca da, afisam un mesaj de eroare si iesim din functie. Daca nu, verificam daca username-ul
si parola au spatii. Daca da, returnam mesaj de eroare si iesim din functie. Daca nu, verificam daca usernameul si parola 
sunt goale, daca da, afisam un mesaj de eroare si iesim din functie. Daca nu, construim un JSON in felul
urmator: initializam un obiect JSON, adaugam username-ul si parola in obiect, si apoi convertim obiectul in string.
Rezultatul il punem in json_string. Apelam `compute_post_request` cu host, port, ruta de inregistrare, tipul de
continut, json_string, cookie (NULL) si JWT token (NULL). Trimitem request-ul la server, si primim raspunsul.
Verificam daca username-ul exista deja, daca da, afisam un mesaj de eroare si iesim din functie. Daca nu, incheiem
conexiunea cu serverul si afisam un mesaj de succes in main.

2. `login` - Aceasta functie este folosita pentru a autentifica un utilizator. Declaram message, response si
sockfd. Inainte de a deschide conexiunea cu serverul, verificam daca cookieul e deja setat, daca da, afisam un mesaj
de eroare si iesim din functie. Daca nu, deschidem conexiunea, luam username-ul si parola de la utilizator, verificam daca 
acestea au caractere invalide, daca da, afisam un mesaj de eroare si iesim din functie. Daca nu, verificam daca username-ul
si parola au spatii. Daca da, returnam mesaj de eroare si iesim din functie. Daca nu, verificam daca usernameul si parola 
sunt goale, daca da, afisam un mesaj de eroare si iesim din functie. Daca nu, construim un JSON
la fel ca in `register_user`, si apelam `compute_post_request` cu host, port, ruta de autentificare, tipul de continut,
json_string, cookie (NULL) si JWT token (NULL). Trimitem request-ul la server, si primim raspunsul. Verificam daca
username-ul si parola sunt corecte si daca exista userul, daca da, setam cookieul si afisam un mesaj de succes in main. 
Daca nu, afisam un mesaj de eroare pentru cazul dat, si iesim din functie.

3. `get_book_access` - Aceasta functie este folosita pentru a accesa o librarie de carti. Declaram message, response si
sockfd. Verificam daca avem cookieul setat, daca nu, afisam un mesaj de eroare si iesim din functie. Verificam apoi daca exista
deja un jwt token, daca da, inseamna ca suntem deja in librarie, si afisam un mesaj de eroare si iesim din functie. 
Daca nu, deschidem conexiunea cu serverul, si apelam `compute_get_request` cu host, port, ruta de accesare 
a librariei, parametrul de query (NULL), cookieul, si JWT token (NULL). Trimitem request-ul la server, si primim raspunsul.
Extragem tokenul JWT din raspuns, si il retinem in variabila globala, ce devine accesul nostru pentru librarie. Incheiem 
conexiunea cu serverul si afisam un mesaj de succes in main.

4. `get_books` - Aceasta functie este folosita pentru a afisa toate cartile din librarie. Declaram message, response si
sockfd. Verificam daca avem acces la librarie, daca nu, afisam un mesaj de eroare si iesim din functie. Daca da, deschidem conexiunea cu serverul, si apelam `compute_get_request` cu host, port, ruta de accesare a cartilor, parametrul de query (NULL), cookieul, si JWT token. Trimitem request-ul la server, si primim raspunsul. Afisam toate cartile din librarie si incheiem conexiunea cu serverul.

5. `get_book` - Aceasta functie este folosita pentru a afisa o carte din librarie. Declaram message, response si sockfd.
Verificam daca avem acces la librarie, daca nu, afisam un mesaj de eroare si iesim din functie. Daca da,
deschidem conexiunea cu serverul, si luam id-ul cartii de la utilizator. Apelam `compute_get_request` cu host, port, ruta
de accesare a cartii, id-ul cartii, cookieul, si JWT token. Trimitem request-ul la server, si primim raspunsul. Daca da, 
verificam daca cartea exista. Daca nu exista, afisam un mesaj de eroare si iesim din functie. Daca da, afisam cartea. Incheiem 
conexiunea cu serverul.

6. `add_book` - Aceasta functie este folosita pentru a adauga o carte in librarie. Declaram message, response si sockfd.
Verificam daca avem acces la librarie, daca nu, afisam un mesaj de eroare si iesim din functie. Daca da, deschidem conexiunea
cu serverul, si luam titlul, autorul, publisherul, genul si numarul de pagini al cartii de la utilizator.
Verificam daca acestea sunt toate categoriile scrise, daca nu, afisam un mesaj de eroare si iesim din functie. Verificam apoi daca numarul de pagini e un numar. Daca nu e, afisam un mesaj de eroare si iesim din functie. Daca da, construim
un JSON in felul urmator: initializam un obiect JSON, adaugam titlul, autorul, publisherul, genul si numarul de pagini in obiect,
si apoi convertim obiectul in string. Rezultatul il punem in json_string. Apelam `compute_post_request` cu host, port, ruta
de adaugare a cartii, tipul de continut, json_string, cookieul si JWT token. Trimitem request-ul la server, si primim raspunsul.
Afisam un mesaj de succes in main si incheiem conexiunea cu serverul.

7. `delete_book` - Aceasta functie este folosita pentru a sterge o carte din librarie. Declaram message, response si sockfd.
Verificam daca avem acces la librarie, daca nu, afisam un mesaj de eroare si iesim din functie. Daca da, deschidem conexiunea cu 
serverul, si luam id-ul cartii de la utilizator. Daca id-ul ala a fost deja sters, se returneaza un mesaj de eroare si iesim din
functie. Daca nu, apelam `compute_delete_request` cu host, port, ruta de stergere a cartii, id-ul cartii, cookieul si JWT token. 
Trimitem request-ul la server, si primim raspunsul. Verificam daca cartea exista, daca nu, afisam un mesaj de eroare si iesim din 
functie. Daca da, salvam id-ul sters, afisam un mesaj de succes in functie si incheiem conexiunea cu serverul.

8. `logout` - Aceasta functie este folosita pentru a dezautentifica un utilizator. Declaram message, response si sockfd.
Verificam daca exista cookieul, daca nu, afisam un mesaj de eroare si iesim din functie. Altfel, deschidem conexiunea cu 
serverul, si apelam `compute_get_request` cu host, port, ruta de dezautentificare, parametrul de query
(NULL), cookieul si JWT token. Trimitem request-ul la server, si primim raspunsul. Stergem cookieul si JWT tokenul, si afisam un 
mesaj de succes in main.

9. `exit` - Aceasta am scris-o in main, pentru a iesi din aplicatie. Daca utilizatorul scrie "exit", se va apela exit(0) si
se va iesi din aplicatie.

10. `main` - In main, am facut un while loop infinit, in care am citit comanda de la utilizator. Daca comanda este "register",
apelam `register_user`. Daca comanda este "login", apelam `login`. Daca comanda este "enter_library", apelam `get_book_access`.
Daca comanda este "get_books", apelam `get_books`. Daca comanda este "get_book", apelam `get_book`. Daca comanda este "add_book",
apelam `add_book`. Daca comanda este "delete_book", apelam `delete_book`. Daca comanda este "logout", apelam `logout`. Daca
comanda este "exit", iesim din aplicatie si din loop. Altfel, continuam while loop-ul.
