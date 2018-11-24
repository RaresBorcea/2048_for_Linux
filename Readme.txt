Durata studiului necesar înțelegerii bibliotecii 'ncurses': ≃ 1.5 zile.
Durata implementării propriu-zise a programului: ≃ 2.5 zile.

Jocul 2048 îneplinește toate cele 5 cerințe ale enunțului. 
Sub forma unui executabil obținut la rularea comenzii 'make', jocul întâmpină utilizatorul cu un meniu și instrucțiuni de navigare.
Meniul prezintă opțiunile: New Game, Resume (disponibilă doar dacă există sesiuni curente) și Quit.
Navigarea în meniu se realizează prin tastele săgeți UP și DOWN.
Odată început un joc nou, utilizatorului i se prezintă tabela de joc și un panou de control.
Controlul jocului se realizează din tastele direcționale (săgeți).
La fiecare combinare dintre două celule, scorul va fi actualizat conform cerinței.
Ora și data vor fi actualizate în timp real.
Dacă timp de 8 secunde nu se efectuează nicio mutare, jocul va efectua mutarea cea mai eficientă, conform cerinței.
Automutarea poate fi apelată oricând prin apăsarea tastei 'A'.
La apăsarea tastei 'U', se va reveni la starea dinaintea ultimei mutări.
Tasta 'Q' va pune în stand-by sesiunea, aducând utilizatorul la meniu.

Funcțiile utilizate pentru fiecare cerință:
Cerința 1:
1. Funcția print_menu este cea care, alături de inițializarea ferestrelor din main, se ocupă de „printarea” în fereastră a opțiunilor din meniu.
   Funcția folosește un vector tip char* pentru opțiunile meniului și face glow pe opțiunea curentă.
2. Alte elemente ASCII afișate pe ecran vor fi scrise în prima parte a funcției main.

Cerința 2:
1. Funcția print_board este cea în care se scrie tabela jocului și a panoului de control + completarea acestuia cu scorul și ora curente.
   Pentru dată și oră au fost folosite biblioteca 'time.h', un întreg tip de date time_t și o structură, șirul final fiind reținut în șirul date_t.
2. Funcția init_board este cea care inițializează matricea din spatele jocului, introducând 2 elemente generate aleator. 
3. Funcția fill_board este cea care „printează” matricea și, eventual, mesajul de câștigare a jocului.
   Se vor folosi culori distincte pentru primele 12 puteri ale lui 2.

Cerința 3:
1. Funcția game este cea de prelucrare propriu-zisă a matricei, funcția care calculează schimbările necesare fiecărei mutări (core-ul programului).
   În funcție de direcția primită ca parametru (de la 0 la 3, de la nord la vest), aceasta va modifica matricea corespunzător.
   Aceasta gestionează și îmbinarea celulelor și verifică după fiecare mutare dacă jocul a fost câștigat (variabila winner).
   Tot aici, prin pointeri, va fi actualizată și variabila de scor.
2. Funcția new_value generează după fiecare mutare validă o valoare aleatoare, atât ca valoare, cât și ca poziție pe tabelă.
3. Funcția valid_move verifică dacă o mutare este validă, aplicând-o întâi pe o matrice test.
   Este folosită și în cadrul funcției automove, deci va funcționa în 2 moduri:
   0 - doar verifică validitatea;
   1 - dacă mutarea este validă, funcția efectuează și actualizarea matricei.

Cerința 4:
   Funcția automove este cea care, cu ajutorul funcției 'select' apelată în main, va efectua singură o mutare în cazul a 8 secunde în care utilizatorul nu interacționează.
   Funcția va calcula cea mai eficientă mutare (cea care eliberează cele mai multe celule - variabila counter).
   Calculul se va efectua prin comparația celor 4 matrice obținute în eventualitatea celor 4 mutări posibile.
   Dacă există mutări valide, funcția va apela celelate funcții necesare actualizării tabelei și ecranului.

Cerința 5:
1. Funcția gameover va fi apelată după fiecare mutare efectată, verificând, prin apelarea valid_move în mode 0, dacă mai există mișcări valide.
2. Funcția gameover_print este cea care, în cazul în care nu mai există mișcări valide, va afișa un mesaj corespunzător.
   Ea va finaliza sesiunea și va întoarce utilizatorul la meniu, după apăsarea încă unei taste.
   S-a ținut cont ca mesajul de OVER să nu se suprapună cu cel de WIN, în cazul afișării amândurora.

BONUS:
1. Folosirea culorilor în cadrul întregului joc și a ASCII Art pentru a îmbunătăți experiența utilizatorului.
2. Folosirea bibliotecii 'panel.h', pentru a avea un control mai bun asupra ferestrelor afișate pe ecran la un anumit moment.
3. Posibilitatea apelării funcției automove oricând la apăsarea tastei 'A'.
4. Funcția backup_undo poate fi apelată prin apăsarea tastei 'U', readucând pe ecran starea anterioară a tabelei de joc, a scorului și a variabilei winner.
   Aceasta va îndeplini două roluri, în funcție de mode:
   0 - modul backup, când cele trei elemente doar sunt salvate;
   1 - modul undo, cand se va face revenirea la starea salvată anterior.
5. Atâta vreme cât terminalul nu se află în modul Full Screen, fereastra executabilului va fi redimensionată automat la dimensiunea optimă.

Pentru mai multe detalii, recomand urmărirea comentariilor care însoțesc fiecare subprogram.
