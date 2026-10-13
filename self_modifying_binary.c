#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

volatile char string_to_modify[100] = "Hello World, i have been modified: 0 times\n";

volatile int current_iteration =1;

//struct modifiable_data {


int main(int argc, char **argv){
    //Har volatile slik at le kompilator ikke optimaliserer bort 100 bytes størrelse
    char file_path[200];
    //readlink leser symbolic path og gjør om til streng
    ssize_t n = readlink("/proc/self/exe", file_path, sizeof(file_path)-1);
    file_path[n] = '\0';
    //Dette funka fjell
    printf("Verdien til min file_path %s\n", file_path);

    printf("%s", string_to_modify);
    //Skal nå lese binary filen sui i guess 
    //Et problem med å lese filen i buffers er hvis man deler opp filen i mindre
    //deler ved buffers er at vi kan kutte vår target streng i 2 aka vi matcher ikke
    //For å løse dette laster vi inn hele filen i en go

    //////////////#############################//////////////////////
    //Hva er neste steg?, det blir: Få den til å modifisere seg selv
    //til noe nytt hver gang uendelig, med nytt navn hver gang.
    //Har også lyst til å endre et tall i programmet sånn fks current iteration

    //Etter jeg har fått til dette så vil jeg endre den slik at den kaller exec på
    //den nye filen med det nye navnet med sleep 10 sek slik at jeg klarer å stoppe den
    //Etter dette vil også få til å slette gamle binaries, slik at jeg har en binary
    //som konstant muterer.

    //Etter dette vil spille av lyd, Vil spille av American on line skhshhhhh lyden
    //Pluss vil åpne masse windows

    //En urelatert idè vil lage window art med å åpne tusen windows, som kryss
    //og kors etc kanskje ascii også

    FILE *my_binary = fopen(file_path, "rb");
    fseek(my_binary, 0, SEEK_END);
    // ftell forteller indeksen til våre seeker aka lengden på filen.
    unsigned int size = ftell(my_binary);
    //Dette her burde gå til starten av filen igjen
    fseek(my_binary, 0,  SEEK_SET);
    char *file_copy = malloc(size);
    //Nå skal vi lese hele filen og putte den i file_copy
    fread(file_copy, 1, size, my_binary);
    fclose(my_binary);

    //skal bruke memmem memory search submemory tydeligvis
    //størrelse på buffer, størrelse på det jeg leter etter, 
    char* location_of_substring = memmem(file_copy, size,
         (const void *)string_to_modify, sizeof(string_to_modify));
    
    char* location_of_substring_in_file = (char *)(location_of_substring - file_copy);
    printf("Start adressen til vår string %p\n", location_of_substring);

    char replacement_string[100] = "Hello World, i have been modified: 1 times\n";

    //Jeg får segfault når jeg kjører den modifiserte binarien.
    for(int i = 0; i<strlen(replacement_string); i++){
        //Det her gir vel ikke mening fordi jeg indekserer så
        //jeg burde vel faktisk bruke loc_in_file
        file_copy[(unsigned long)location_of_substring_in_file+i] = replacement_string[i];
    }
    //Hodet mitt er litt kokt, skal tilbake til self modifying etterpå.
    //venta må kanse bruke strcompare
    //waht the helly
    //ser hva som er feil..... min file path er større lissom
    printf("verdi til file path: %s\n", file_path);
    if (0!=(strcmp(file_path, "/home/eskild/Documents/Høst_2026/replicating_worm/modified_binary.exe"))){
        printf("Lager en ny binary \n");

        FILE *new_binary = fopen("./modified_binary.exe", "w+");
        ssize_t n_new = fwrite(file_copy, 1, size, new_binary);
        fclose(new_binary);
    }

    return 0;
}