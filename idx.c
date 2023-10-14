#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Length
#define NAME 170
#define ID 160
#define CATEGORY 30
#define INSTALLS 15
#define FREE 5
#define SIZE 18
#define LAST_UPDATED 9
#define CONTENT_RATING 15
#define TOTAL NAME+ID+CATEGORY+INSTALLS+FREE+SIZE+LAST_UPDATED+CONTENT_RATING

struct Record {
    int number;
    char name[NAME];
    char id[ID];
    char category[CATEGORY];
    float rating;
    int rating_count;
    char installs[INSTALLS];
    char free[FREE];
    char size[SIZE];
    char last_updated[LAST_UPDATED];
    char content_rating[CONTENT_RATING];
};

struct Index {
    int number;
    long offset;
};

struct RecordNumberNode {
    int number;
    struct RecordNumberNode *next;
};

struct TreeNode {
    char category[CATEGORY];
    struct RecordNumberNode *recordNumbers;
    struct TreeNode *left;
    struct TreeNode *right;
    int height;
};

struct RatingIndexHashNode {
    int hash;
    int count;
    struct RatingIndexNode *firts;
    struct RatingIndexHashNode *next;
};

struct RatingIndexNode {
    int number;
    float rating;
    struct RatingIndexNode *last;
    struct RatingIndexNode *next;
};


// Global Variables
struct RatingIndexHashNode *hashIndex = NULL;


//---------------------------------------------------------------------------------------------------------------------------------------------
// Binary Search
void searchByNumber(const char *binaryFile, int targetNumber) {
    FILE *binary = fopen(binaryFile, "rb");

    if (binary == NULL) {
        perror("Error opening the binary file");
        exit(1);
    }

    struct Record record;
    int comparisons = 0;
    int found = 0;

    fseek(binary, 0, SEEK_END);
    long fileSize = ftell(binary); // Get the file size in bytes
    long recordSize = sizeof(struct Record);

    long start = 0;
    long end = (fileSize / recordSize) - 1;

    while (start <= end) {
        long middle = (start + end) / 2;
        fseek(binary, middle * recordSize, SEEK_SET); // Move to the middle of the file

        fread(&record, sizeof(struct Record), 1, binary);
        comparisons++;

        printf("Number: %d\n", record.number);
        printf("Comparisons: %d\n\n", comparisons);

        if (record.number == targetNumber) {
            found = 1;
            break;
        } else if (record.number < targetNumber) {
            start = middle + 1;
        } else {
            end = middle - 1;
        }
    }

    if (found) {
        printf("Record found:\n");
        printf("Number: %d\n", record.number);
        printf("Name: %s\n", record.name);
        // Print other fields of the struct as needed
        printf("Comparisons: %d\n", comparisons);
    } else {
        printf("Record with number %d not found.\n", targetNumber);
    }

    fclose(binary);
}


//---------------------------------------------------------------------------------------------------------------------------------------------
//Arquivo binário
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
    char header[TOTAL];
    fgets(header, sizeof(header), csvFile);
    
    char line[TOTAL];
    
    // Read and convert each line from the CSV file into a struct and write it to the binary file
    struct Record record;
    while (fgets(line, sizeof(line), csvFile)) {
        sscanf(line, "%d,%[^,],%[^,],%[^,],%.1f,%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",
                  &record.number, record.name, record.id, record.category, &record.rating, &record.rating_count,
                  record.installs, record.free, record.size, record.last_updated, record.content_rating);
        
        fillWithSpaces(record.name, NAME);
        fillWithSpaces(record.id, ID);
        fillWithSpaces(record.category, CATEGORY);
        fillWithSpaces(record.installs, INSTALLS);
        fillWithSpaces(record.free, FREE);
        fillWithSpaces(record.size, SIZE);
        fillWithSpaces(record.last_updated, LAST_UPDATED);
        fillWithSpaces(record.content_rating, CONTENT_RATING);
        
        //TEST
        /*teste = 0
        if(teste <5){
            printf("\nNum=%d\n Name=%s\n Id=%s\n Category=%s\n Rating=%f\n RatingCount=%d\n Installs=%s\n Free=%s\n Size=%s\n LastUpdate=%s\n Content=%s\n", record.number, record.name, record.id, record.category, record.rating, record.rating_count, record.installs, record.free, record.size, record.last_updated, record.content_rating);
            teste++;
        }*/

        fwrite(&record, sizeof(struct Record), 1, binFile);
    }

    fclose(csvFile);
    fclose(binFile);
    printf("Binary file created successfully.\n");
}


//---------------------------------------------------------------------------------------------------------------------------------------------
//Arquivo de indice baseado no campo "Number"
void createIndexFile(const char *binFileName, const char *indexFileName) {
    FILE *binFile = fopen(binFileName, "rb");
    if (!binFile) {
        printf("Error opening the binary file.\n");
        return;
    }

    FILE *indexFile = fopen(indexFileName, "wb");
    if (!indexFile) {
        printf("Error creating the index file.\n");
        fclose(binFile);
        return;
    }

    struct Record record;
    struct Index index;

    long binOffset = 0;

    while (fread(&record, sizeof(struct Record), 1, binFile)) {
        index.number = record.number;
        index.offset = binOffset;
        fwrite(&index, sizeof(struct Index), 1, indexFile);

        binOffset += sizeof(struct Record);
    }

    fclose(binFile);
    fclose(indexFile);

    printf("Index file created successfully.\n");
}
//Mostrar arquivo de índice
void showIndexFile(const char *indexFileName) {
    FILE *indexFile = fopen(indexFileName, "rb");
    if (!indexFile) {
        printf("Error opening the index file.\n");
        return;
    }

    struct Index index;

    while (fread(&index, sizeof(struct Index), 1, indexFile)) {
        printf("Number: %d, Offset: %ld\n", index.number, index.offset);
    }

    fclose(indexFile);
}

//---------------------------------------------------------------------------------------------------------------------------------------------
//AVL
// Função auxiliar para calcular o máximo de dois inteiros
int max(int a, int b) {
    return (a > b) ? a : b;
}
// Função auxiliar para obter a altura de um nodo da árvore
int height(struct TreeNode *node) {
    if (node == NULL) {
        return 0;
    }
    return node->height;
}
// Função para criar um novo nodo da árvore
struct TreeNode *newTreeNode(char category[CATEGORY]) {
    struct TreeNode *node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    strcpy(node->category, category);
    node->recordNumbers = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}
// Função para realizar uma rotação à direita
struct TreeNode *rightRotate(struct TreeNode *y) {
    struct TreeNode *x = y->left;
    struct TreeNode *T2 = x->right;

    // Realiza a rotação
    x->right = y;
    y->left = T2;

    // Atualiza alturas
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}
// Função para realizar uma rotação à esquerda
struct TreeNode *leftRotate(struct TreeNode *x) {
    struct TreeNode *y = x->right;
    struct TreeNode *T2 = y->left;

    // Realiza a rotação
    y->left = x;
    x->right = T2;

    // Atualiza alturas
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}
// Função para obter o fator de balanceamento de um nodo
int getBalance(struct TreeNode *node) {
    if (node == NULL) {
        return 0;
    }
    return height(node->left) - height(node->right);
}
// Função para inserir um número de registro em uma categoria na árvore AVL
struct RecordNumberNode *insertRecordNumber(struct RecordNumberNode *node, int number) {
    if (node == NULL) {
        struct RecordNumberNode *newNode = (struct RecordNumberNode *)malloc(sizeof(struct RecordNumberNode));
        newNode->number = number;
        newNode->next = NULL;
        return newNode;
    }

    node->next = insertRecordNumber(node->next, number);
    return node;
}
// Função para inserir um nodo na árvore AVL
struct TreeNode *insertTreeNode(struct TreeNode *node, char category[CATEGORY], int number) {
    // Realiza a inserção normal de um nodo de árvore binária de busca
    if (node == NULL) {
        return newTreeNode(category);
    }

    if (strcmp(category, node->category) < 0) {
        node->left = insertTreeNode(node->left, category, number);
    } else if (strcmp(category, node->category) > 0) {
        node->right = insertTreeNode(node->right, category, number);
    } else {
        // A categoria já existe, adicione o número de registro à lista
        node->recordNumbers = insertRecordNumber(node->recordNumbers, number);
        return node;
    }

    // Atualiza a altura do nodo atual
    node->height = 1 + max(height(node->left), height(node->right));

    // Obter o fator de balanceamento deste nodo para verificar se ele se tornou desequilibrado
    int balance = getBalance(node);

    // Casos de desequilíbrio

    // Esquerda-Esquerda
    if (balance > 1 && strcmp(category, node->left->category) < 0) {
        return rightRotate(node);
    }

    // Direita-Direita
    if (balance < -1 && strcmp(category, node->right->category) > 0) {
        return leftRotate(node);
    }

    // Esquerda-Direita
    if (balance > 1 && strcmp(category, node->left->category) > 0) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    // Direita-Esquerda
    if (balance < -1 && strcmp(category, node->right->category) < 0) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}
// Função para realizar uma travessia em ordem da árvore e listar categorias e números
void inOrderTraversal(struct TreeNode *root) {
    if (root != NULL) {
        inOrderTraversal(root->left);
        printf("Category: %s\n", root->category);
        struct RecordNumberNode *recordNode = root->recordNumbers;
        while (recordNode != NULL) {
            printf("  Number: %d\n", recordNode->number);
            recordNode = recordNode->next;
        }
        inOrderTraversal(root->right);
    }
}
// Função para pesquisar por uma categoria na árvore AVL e retornar a lista de números
struct RecordNumberNode *searchCategory(struct TreeNode *root, char category[CATEGORY]) {
    if (root == NULL) {
        return NULL;
    }

    int cmpResult = strcmp(category, root->category);

    if (cmpResult == 0) {
        return root->recordNumbers;
    } else if (cmpResult < 0) {
        return searchCategory(root->left, category);
    } else {
        return searchCategory(root->right, category);
    }
}
//Criar AVL
struct TreeNode *createAVLFromBinaryFile(const char *binaryFile) {
    struct TreeNode *root = NULL;
    FILE *binFile = fopen(binaryFile, "rb");

    if (!binFile) {
        printf("Error opening the binary file.\n");
        return NULL; // Retorne NULL em caso de erro
    }

    struct Record record;
    while (fread(&record, sizeof(struct Record), 1, binFile)) {
        //Limite de memória
        if(record.number<100000){
            root = insertTreeNode(root, record.category, record.number);
        }
    }

    printf("AVL created successfully.\n");

    fclose(binFile);
    return root;
}

//---------------------------------------------------------------------------------------------------------------------------------------------
//Indice baseado no campo "Rating", em memória
//Cria um novo nodo de cabeçalho
struct RatingIndexHashNode * newHashNode(int hash, struct RatingIndexHashNode *next){
    struct RatingIndexHashNode *new = (struct RatingIndexHashNode*)malloc(sizeof(struct RatingIndexHashNode));
    new->hash = hash;
    new->next = next;
    new->count = 0;
    new->firts = NULL;

    return new;
}
//Cria um novo nodo para o registro
struct RatingIndexNode * newRatingNode(int number, float rating, struct RatingIndexNode *last) {
    struct RatingIndexNode *new = (struct RatingIndexNode*)malloc(sizeof(struct RatingIndexNode));
    new->number = number;
    new->rating = rating;
    new->last = last;
    new->next = NULL;

    return new;
}
//Grea o HASH
int getHash(struct Record record){
    return record.rating*10;
}
//Cria o índice de Rating em memória
void createIndexMemoryByRating(const char *binFileName) {
    FILE *binFile = fopen(binFileName, "rb");
    if (!binFile) {
        printf("Error opening the binary file.\n");
        return;
    }

    struct Record record;

    // int i = 0; /*DEBUG*/

    while (
        // i<10000 &&  /*DEBUG*/
        fread(&record, sizeof(struct Record), 1, binFile)
    ) {
        
        // i++; /*DEBUG*/
        
        int hash = getHash(record);

        if (hashIndex == NULL) {
            hashIndex = newHashNode(hash, NULL);
            hashIndex->firts = newRatingNode(record.number, record.rating, NULL);
            hashIndex->count = 1;
        }
        else {
            struct RatingIndexHashNode *cur = hashIndex;
            while(cur->next != NULL && cur->hash < hash && (cur->next)->hash < hash)
                cur = cur->next;

            if (cur->next != NULL && (cur->next)->hash == hash)
                cur = cur->next;

            if (cur->hash == hash) {
                struct RatingIndexNode *curApp = cur->firts;
                cur->count++;

                while (curApp->next != NULL)
                    curApp = curApp->next;

                curApp->next = newRatingNode(record.number, record.rating, curApp);
            }
            else {
                cur->next = newHashNode(hash, cur->next);
                (cur->next)->firts = newRatingNode(record.number, record.rating, NULL);
                (cur->next)->count = 1;
            }
        }
    }

    fclose(binFile);

    printf("Index created successfully.\n");
}
void printIndexMemoryByRating() {
    struct RatingIndexHashNode *curHash = hashIndex;
    while (curHash != NULL) {
        printf("\nHash: %d - Qtd: %d\n", curHash->hash, curHash->count);
        
        struct RatingIndexNode *cur = curHash->firts;
        while(cur != NULL) {
            printf("%d (%.1f) | ", cur->number, cur->rating);

            cur = cur->next;
        }

        curHash = curHash->next;
    }
}


//---------------------------------------------------------------------------------------------------------------------------------------------
int main() {
    const char *textFile = "apps.csv";
    const char *binaryFile = "apps.bin";
    // const char *indexNumberFile = "idxnumber.bin";
    

    // //Criação arquivo binário
    // createBinaryFile(textFile, binaryFile);

    //Criar arquivo sequencial indexado campo numero
    //createIndexFile(binaryFile, indexNumberFile);
    //showIndexFile(indexNumberFile);

    
    
    // //Criar AVL
    // struct TreeNode *root = createAVLFromBinaryFile(binaryFile);


    // //Procurar números em uma categoria específica
    // char searchedCategory[CATEGORY] = "Education                    ";
    // struct RecordNumberNode *categoryNumbers = searchCategory(root, searchedCategory);
    // if (categoryNumbers) {
    //     printf("Numbers in category '%s':\n", searchedCategory);
    //     struct RecordNumberNode *current = categoryNumbers;
    //     while (current != NULL) {
    //         printf("%d\n", current->number);
    //         current = current->next;
    //     }
    // } else {
    //     printf("Category '%s' not found.\n", searchedCategory);
    // }
    

    // createIndexMemoryByRating(binaryFile);
    // printIndexMemoryByRating();

    return 0;
}


//---------------------------------------------------------------------------------------------------------------------------------------------
/*Pesquisa binária pelo nome

    searchByName(binaryFile, "Name");

void searchByName(const char *binaryFile, const char *searchName) {
    FILE *binary = fopen(binaryFile, "rb");

    if (binary == NULL) {
        perror("Error opening the binary file");
        exit(1);
    }

    struct Record record;
    int comparisons = 0;
    int found = 0;

    fseek(binary, 0, SEEK_END); // Move the cursor to the end of the file
    long fileSize = ftell(binary); // Get the file size in bytes
    long recordSize = sizeof(struct Record);

    long start = 0;
    long end = (fileSize / recordSize) - 1;

    while (start <= end) {
        long middle = (start + end) / 2;
        fseek(binary, middle * recordSize, SEEK_SET); // Move to the middle of the file

        fread(&record, sizeof(struct Record), 1, binary);
        comparisons++;

        printf("Name: %s\n", record.name);
        printf("Comparisons: %d\n\n", comparisons);

        int comparison = strncmp(record.name, searchName, strlen(searchName));

        if (comparison == 0) {
            found = 1;
            break;
        } else if (comparison < 0) {
            start = middle + 1;
        } else {
            end = middle - 1;
        }
    }

    if (found) {
        printf("Record found:\n");
        printf("Name: %s\n", record.name);
        printf("Genre: %s\n", record.genre);
        printf("Comparisons: %d\n", comparisons);
    } else {
        printf("Record not found.\n");
    }

    fclose(binary);
}
*/