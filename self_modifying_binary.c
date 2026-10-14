#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//Trenger denne for chmod
#include <sys/stat.h>

int number_of_evolutions = 100;
int evolution_speed = 500; //Antall millisekunder per evolution.

//Det her var ikke min idé bare så det er sagt, men poenget er at man kan finne
//current iteration ved å ta offset fra strengen som er lett å finne.
struct modifiable_data {
    char message[100];
    int current_iteration;
    char full_file_path[200];
};

//Har volatile slik at le kompilator ikke optimaliserer bort 100 bytes størrelse
volatile struct modifiable_data mod_data = {"Hello World, number of times modified:",
     1, ""};


int main(int argc, char **argv){
    //Hvis programmet blir kalt selvom det har nådd max iterations
    //så betyr det at det ikke kalte seg selv, men ble kalt eksplisitt
    //Hvis dette er tilfelel så resetter vi current_iteration
    if(mod_data.current_iteration == number_of_evolutions) mod_data.current_iteration=1;

    //Leser argumenter hvis de eksisterer
    if(argc == 3){
        number_of_evolutions = atoi(argv[1]);
        evolution_speed = atoi(argv[2]);
    }

    //Må caste til vanlig char pointer fordi den gir warning med volatile
    ssize_t n = readlink("/proc/self/exe", (char*)mod_data.full_file_path,
         sizeof(mod_data.full_file_path)-1);
    mod_data.full_file_path[n] = '\0';

    printf("%s %d\n", mod_data.message, mod_data.current_iteration);

    //deler ved buffers er at vi kan kutte vår target streng i 2 aka vi matcher ikke
    //For å løse dette laster vi inn hele filen i en go
    FILE *my_binary = fopen((char*)mod_data.full_file_path, "rb");
    fseek(my_binary, 0, SEEK_END);
    // ftell forteller indeksen til våre seeker aka lengden på filen.
    unsigned int size = ftell(my_binary);
    //Går tilbake til starten av filen
    fseek(my_binary, 0,  SEEK_SET);
    char *file_copy = malloc(size);
    //Leser hele filen og putter den i file_copy
    fread(file_copy, 1, size, my_binary);
    fclose(my_binary);

    //skal bruke memmem memory search submemory tydeligvis
    //størrelse på buffer, størrelse på det jeg leter etter, 
    char* modifiable_data_adr = memmem(file_copy, size,
         (const void *)mod_data.message, sizeof(mod_data.message));
    
    char* mod_data_file_adr = (char *)(modifiable_data_adr - file_copy);
    printf("Start adressen til mod data i binarien %p\n", mod_data_file_adr);

    //Erstatter message i vår kopi av filen, det her kan være hva som heslt
    char new_message[100] = "Hello World, number of times modified";
    for(int i = 0; i<strlen(new_message); i++){
        file_copy[(unsigned long)mod_data_file_adr+i] = new_message[i];
    }

    //Oppdaterer current iteration i vår kopi av filen
    //Bruker memcopy siden jeg gjorde file copy til en char pointer, men skal int
    mod_data.current_iteration++;
    memcpy(modifiable_data_adr+sizeof(new_message), (int*)&mod_data.current_iteration, 4);

    //Oppdaterer til ny filepath
    //Hva vil jeg at programmet skal hete
    //worm-strain-x901jf.exe    Den er ganske fire så //worm-strain-xKKKKK.exe
    //eller hele tilfeldig: worm-strain-oawfj2.exe hmmm hva er kulest?
    
    //Ascii verdier mellom 48-57 er tall mellom 97 og 122 er små bokstaver
    //den totale rangen blir 10+25 = 35.
    char name_tag[7];
    srandom(getpid());
    for(int i = 0; i<6; i++){
        int random_number = random();
        int random_character = random_number%35;
        //Rangen for tall
        if(random_character <10){
            random_character += 48;
        }
        //blir +87 for å få rangen av små bokstaver
        else random_character +=87;
        name_tag[i] = (char) random_character; 
    }
    name_tag[6] = '\0';
    
    char new_file_name[40];
    sprintf(new_file_name, "worm-strain-%s.exe",name_tag);

    //string rear char finner indeksen til siste instans av en char
    char *pointer_start_name = strrchr((char *)mod_data.full_file_path, '/');
    int file_name_index = (pointer_start_name + 1) - mod_data.full_file_path;

    //Kan bruke insane funksjon unlink(path) for å fjerne min egen binary fil
    //samtidig som programmet fortsetter å kjøre, tror ikke det funker på windows da
    //Gjør dette før vi oppdaterer file path
    unlink((char *)mod_data.full_file_path);

    for(int i = 0; i<strlen(new_file_name); i++){
        mod_data.full_file_path[file_name_index +i] = new_file_name[i];
    }
    printf("verdi til file path: %s\n", mod_data.full_file_path);
    
    //Oppdaterer den nye fullpathen (filename) i vår kopi av filen
    memcpy(modifiable_data_adr+sizeof(mod_data.message)+ sizeof(mod_data.current_iteration),
    (char *)mod_data.full_file_path, sizeof(mod_data.full_file_path));

    printf("Lager en ny binary \n\n");
    FILE *new_binary = fopen((char *)mod_data.full_file_path, "w+");
    ssize_t n_new = fwrite(file_copy, 1, size, new_binary);
    chmod((char *)mod_data.full_file_path, 0777);
    fclose(new_binary);

    usleep(1000 * evolution_speed);
    pid_t child_pid = 0;
    if(mod_data.current_iteration < number_of_evolutions){
        child_pid = fork();
        if(child_pid == 0){
            execl((char *)mod_data.full_file_path, new_file_name, argv[1], argv[2], (char*)NULL);
        }
    }

    return 0; 
}