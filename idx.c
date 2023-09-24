#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Length
#define NAME 170
#define ID 160
#define CATEGORY 30
#define RATING 3
#define RATING_COUNT 10
#define INSTALLS 15
#define FREE 5
#define SIZE 18
#define LAST_UPDATED 9
#define CONTENT_RATING 15
#define TOTAL NAME+ID+CATEGORY+RATING+RATING_COUNT+INSTALLS+FREE+SIZE+LAST_UPDATED+CONTENT_RATING

struct Record {
    int number;
    char name[NAME];
    char id[ID];
    char category[CATEGORY];
    char rating[RATING];
    char rating_count[RATING_COUNT];
    char installs[INSTALLS];
    char free[FREE];
    char size[SIZE];
    char last_updated[LAST_UPDATED];
    char content_rating[CONTENT_RATING];
};

void fillWithSpaces(char field[], int size) {
    size_t field_len = strlen(field);
    for (size_t i = field_len; i < size; i++) {
        field[i] = ' ';
    }
    field[size - 1] = '\0';
}

void createBinaryFile(const char *csvFileName, const char *binFileName) {
    FILE *csvFile = fopen(csvFileName, "r");
    if (!csvFile) {
        printf("Error opening the CSV file.\n");
        return;
    }

    FILE *binFile = fopen(binFileName, "wb");
    if (!binFile) {
        printf("Error creating the binary file.\n");
        fclose(csvFile);
        return;
    }

    // Read the CSV file header (ignored)
    char header[TOTAL]; // Big enough to store the header
    puts(fgets(header, sizeof(header), csvFile)); //Showing header

    char line[TOTAL + 2]; // +2 for comma and newline
    
    //TEST
    int teste = 0;
    
    // Read and convert each line from the CSV file into a struct and write it to the binary file
    struct Record record;
    while (fgets(line, sizeof(line), csvFile)) {
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]",
                  &record.number, record.name, record.id, record.category, record.rating, record.rating_count,
                  record.installs, record.free, record.size, record.last_updated, record.content_rating);
        
        fillWithSpaces(record.name, NAME);
        fillWithSpaces(record.id, ID);
        fillWithSpaces(record.category, CATEGORY);
        fillWithSpaces(record.rating, RATING);
        fillWithSpaces(record.rating_count, RATING_COUNT);
        fillWithSpaces(record.installs, INSTALLS);
        fillWithSpaces(record.free, FREE);
        fillWithSpaces(record.size, SIZE);
        fillWithSpaces(record.last_updated, LAST_UPDATED);
        fillWithSpaces(record.content_rating, CONTENT_RATING);
        

        //TEST
        if(teste == 0){
            printf("\n%d\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", record.number, record.name, record.id, record.category, record.rating, record.rating_count, record.installs, record.free, record.size, record.last_updated, record.content_rating);
            teste++;
        }

        fwrite(&record, sizeof(struct Record), 1, binFile);
    }

    fclose(csvFile);
    fclose(binFile);
    printf("Binary file created successfully.\n");
}

int main() {
    const char *textFile = "apps.csv";
    const char *binaryFile = "apps.bin";

    createBinaryFile(textFile, binaryFile);

    return 0;
}
