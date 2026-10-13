#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv){
    //Må gjøre et OS call for å lage en ny proc
    pid_t my_id = 0;
    pid_t child_id = 0;
    pid_t parent_id = 0;

    //Claude sier det er en unidirectional pipe så idk mayyyyyne
    //tydeligvis er fd[1] write end og fd[0] read end
    int fd[2];
    pipe(fd);

    child_id = fork();
    my_id = getpid();

    parent_id = getppid();

    printf("Hei fra proc: %d\t my parent %d \t my child %d\n", my_id, parent_id, child_id);

    
    //Vil bruke pipe for å sende meldinger mellom prosessor den tar inn fd
    //Kan bruke 2 pipes for å gå fram og tilbake.
    if(child_id == 0){
        //Nå er vi i child proc, vi vil at child proc skal lese
        close(fd[1]);
        char buffer[100];
        //read returnerer 0 når end of file, tar len-1 fordi vi setter inn \0
        sleep(1);
        ssize_t n = read(fd[0], buffer, sizeof(buffer)-1);
        printf("verdi til n %ld \n", n);
        buffer[sizeof(buffer)-1] = '\n';
        printf("Jeg er child, og jeg fikk %s \n", buffer);
        close(fd[0]);
        //Hva gjør exit, dette må jeg spørre Claude om
        //fflush(stdout);
        //_exit(0); //_exit flusher ikke stdout buffer som tydeligvis er vanlig
        //fordi den arver buffer fra parent etc etc.
    }
    if(child_id != 0){
        //Nå er vi i parentProc
        //Lukker read bufferen
        close(fd[0]);
        char myMessage[100] = "Inshallah et algo";
        ssize_t amount_written = write(fd[1], myMessage, 100);
        close(fd[1]); //Dette signaliserer end of file til child procen.
        wait(NULL); //Dette venter tydeligvis på at child prosessen er ferdig
    }
    //Okay skal nå skrive seg selv til en ny fil.
    FILE *file1 = fopen("./test_text.txt", "w+");
    char eksempel_tekst[100] = "Dette er en eksempeltekst i guess bro.";
    fwrite(eksempel_tekst, 1, sizeof(eksempel_tekst), file1);
    fclose(file1);
    return 0;
}