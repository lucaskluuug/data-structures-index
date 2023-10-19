#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LIMIT 100000

// Length
#define NAME 170
#define ID 160
#define CATEGORY 30
#define INSTALLS 15
#define FREE 5
#define SIZE 18
#define LAST_UPDATED 9
#define CONTENT_RATING 15
#define TOTAL NAME + ID + CATEGORY + INSTALLS + FREE + SIZE + LAST_UPDATED + CONTENT_RATING

// Structs
struct Record
{
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

struct Index
{
    int number;
    long offset;
};

struct RecordNumberNode
{
    int number;
    struct RecordNumberNode *next;
};

struct TreeNode
{
    char category[CATEGORY];
    struct RecordNumberNode *recordNumbers;
    struct TreeNode *left;
    struct TreeNode *right;
    int height;
};

struct RatingIndexHashNode
{
    int hash;
    int count;
    struct RatingIndexNode *firts;
    struct RatingIndexHashNode *next;
};

struct RatingIndexNode
{
    int number;
    float rating;
    struct RatingIndexNode *last;
    struct RatingIndexNode *next;
};

struct IndexName
{
    char name[NAME];
    long offset;
};

struct IndexNameList
{
    struct IndexName *ix;
    struct IndexNameList *next;
};

// Global Variables
struct RatingIndexHashNode *hashIndex = NULL;

//---------------------------------------------------------------------------------------------------------------------------------------------
// Fill with spaces to complete string
void fillWithSpaces(char field[], int size)
{
    size_t field_len = strlen(field);
    for (size_t i = field_len; i < size; i++)
    {
        field[i] = ' ';
    }
    field[size - 1] = '\0';
}

// Create Binary file
void createBinaryFile(const char *csvFileName, const char *binFileName)
{
    FILE *csvFile = fopen(csvFileName, "r");
    if (!csvFile)
    {
        printf("Error opening the CSV file.\n");
        return;
    }

    FILE *binFile = fopen(binFileName, "wb");
    if (!binFile)
    {
        printf("Error creating the binary file.\n");
        fclose(csvFile);
        return;
    }

    // Ignore header
    char header[TOTAL];
    fgets(header, sizeof(header), csvFile);

    char line[TOTAL];

    // Read and convert each line from the CSV file into a struct and write it to the binary file
    struct Record record;
    while (fgets(line, sizeof(line), csvFile))
    {
        sscanf(line, "%d,%[^,],%[^,],%[^,],%f,%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",
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

        fwrite(&record, sizeof(struct Record), 1, binFile);
    }

    fclose(csvFile);
    fclose(binFile);
    printf("Binary file created successfully.\n");
}

//---------------------------------------------------------------------------------------------------------------------------------------------
// Binary Search by number field
int searchByNumber(const char *binaryFile, int targetNumber, int structType)
{
    FILE *file = fopen(binaryFile, "rb");

    if (file == NULL)
    {
        perror("Error opening the binary file");
        exit(1);
    }

    void *record = NULL;
    int recordSize = 0;

    // This is used because the structs Record and Index are different
    if (structType == 1)
    {
        struct Record tempRecord;
        recordSize = sizeof(struct Record);
        record = &tempRecord;
    }
    else if (structType == 2)
    {
        struct Index tempIndex;
        recordSize = sizeof(struct Index);
        record = &tempIndex;
    }

    int comparisons = 0;
    int found = 0;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    long start = 0;
    long end = (fileSize / recordSize) - 1;

    while (start <= end)
    {
        long middle = (start + end) / 2;
        fseek(file, middle * recordSize, SEEK_SET);
        fread(record, recordSize, 1, file);
        comparisons++;

        if (structType == 1)
        {
            struct Record *record1 = (struct Record *)record;
            if (record1->number == targetNumber)
            {
                found = 1;
                fclose(file);
                return middle;
            }
            else if (record1->number < targetNumber)
            {
                start = middle + 1;
            }
            else
            {
                end = middle - 1;
            }
        }
        else if (structType == 2)
        {
            struct Index *record2 = (struct Index *)record;
            if (record2->number == targetNumber)
            {
                found = 1;
                fclose(file);
                return record2->offset;
            }
            else if (record2->number < targetNumber)
            {
                start = middle + 1;
            }
            else
            {
                end = middle - 1;
            }
        }
    }

    fclose(file);

    return -1; // Register not found
}

// Search by Name field
int searchByName(const char *indexFileName, char *name)
{
    FILE *binFile = fopen(indexFileName, "rb");
    if (!binFile)
    {
        printf("Error opening the binary file.\n");
        return 0;
    }

    fillWithSpaces(name, NAME);

    struct IndexName record;
    long recordSize = sizeof(struct IndexName);

    while (fread(&record, sizeof(struct IndexName), 1, binFile))
    {
        if (strcmp(record.name, name) == 0)
        {
            fclose(binFile);
            return record.offset;
        }
    }

    fclose(binFile);
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------------------------------
// Index file based on field "Number"
void createIndexFile(const char *binFileName, const char *indexFileName)
{
    FILE *binFile = fopen(binFileName, "rb");
    if (!binFile)
    {
        printf("Error opening the binary file.\n");
        return;
    }

    FILE *indexFile = fopen(indexFileName, "wb");
    if (!indexFile)
    {
        printf("Error creating the index file.\n");
        fclose(binFile);
        return;
    }

    struct Record record;
    struct Index index;

    long binOffset = 0;

    while (fread(&record, sizeof(struct Record), 1, binFile))
    {
        index.number = record.number;
        index.offset = binOffset;
        fwrite(&index, sizeof(struct Index), 1, indexFile);

        binOffset += sizeof(struct Record);
    }

    fclose(binFile);
    fclose(indexFile);

    printf("Index file created successfully.\n");
}

// Show index file "Number" - Used for test
void showIndexFile(const char *indexFileName)
{
    FILE *indexFile = fopen(indexFileName, "rb");
    if (!indexFile)
    {
        printf("Error opening the index file.\n");
        return;
    }

    struct Index index;

    while (fread(&index, sizeof(struct Index), 1, indexFile))
    {
        printf("Number: %d, Offset: %ld\n", index.number, index.offset);
    }

    fclose(indexFile);
}

//---------------------------------------------------------------------------------------------------------------------------------------------
// AVL - https://www.programiz.com/dsa/avl-tree
// Calculate the max between two integers
int max(int a, int b)
{
    return (a > b) ? a : b;
}

// Get node height
int height(struct TreeNode *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->height;
}

// Create a new tree note
struct TreeNode *newTreeNode(char category[CATEGORY])
{
    struct TreeNode *node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    strcpy(node->category, category);
    node->recordNumbers = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

// Right rotate
struct TreeNode *rightRotate(struct TreeNode *y)
{
    struct TreeNode *x = y->left;
    struct TreeNode *auxTreeNode = x->right;

    // Rotate
    x->right = y;
    y->left = auxTreeNode;

    // Update heights
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

// Left rotate
struct TreeNode *leftRotate(struct TreeNode *x)
{
    struct TreeNode *y = x->right;
    struct TreeNode *auxTreeNode = y->left;

    // Rotate
    y->left = x;
    x->right = auxTreeNode;

    // Update heights
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

// Insert a number record into the list of a category in AVL tree
struct RecordNumberNode *insertRecordNumber(struct RecordNumberNode *node, int number)
{
    if (node == NULL)
    {
        struct RecordNumberNode *newNode = (struct RecordNumberNode *)malloc(sizeof(struct RecordNumberNode));
        newNode->number = number;
        newNode->next = NULL;
        return newNode;
    }

    node->next = insertRecordNumber(node->next, number);
    return node;
}

// Insert node into AVL
struct TreeNode *insertTreeNode(struct TreeNode *node, char category[CATEGORY], int number)
{
    // Normal insertion of a binary search tree node
    if (node == NULL)
    {
        return newTreeNode(category);
    }

    if (strcmp(category, node->category) < 0)
    {
        node->left = insertTreeNode(node->left, category, number);
    }
    else if (strcmp(category, node->category) > 0)
    {
        node->right = insertTreeNode(node->right, category, number);
    }
    else
    {
        // Category already exists, add to list
        node->recordNumbers = insertRecordNumber(node->recordNumbers, number);
        return node;
    }

    // Update current note height
    node->height = 1 + max(height(node->left), height(node->right));

    // Get balance to verify if node became unbalanced
    int balance;
    if (node == NULL)
    {
        balance = 0;
    }
    else{
        balance = height(node->left) - height(node->right);
    }

    // Unbalanced cases
    // Left-Left
    if (balance > 1 && strcmp(category, node->left->category) < 0)
    {
        return rightRotate(node);
    }
    // Right-Right
    if (balance < -1 && strcmp(category, node->right->category) > 0)
    {
        return leftRotate(node);
    }
    // Left-Right
    if (balance > 1 && strcmp(category, node->left->category) > 0)
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    // Right-Left
    if (balance < -1 && strcmp(category, node->right->category) < 0)
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

// Search an specific category into AVL and return numbers list
struct RecordNumberNode *searchCategory(struct TreeNode *root, char category[CATEGORY])
{
    if (root == NULL)
    {
        return NULL;
    }

    int cmpResult = strcmp(category, root->category);

    if (cmpResult == 0)
    {
        // Return the beginning of list 
        return root->recordNumbers;
    }
    else if (cmpResult < 0)
    {
        return searchCategory(root->left, category);
    }
    else
    {
        return searchCategory(root->right, category);
    }
}

// Create AVL
struct TreeNode *createAVL(const char *binaryFile)
{
    struct TreeNode *root = NULL;
    FILE *binFile = fopen(binaryFile, "rb");

    if (!binFile)
    {
        printf("Error opening the binary file.\n");
        return NULL;
    }

    struct Record record;
    while (fread(&record, sizeof(struct Record), 1, binFile))
    {
        // Memory limit
        if (record.number < LIMIT)
        {
            root = insertTreeNode(root, record.category, record.number);
        }
    }

    printf("AVL created successfully.\n");

    fclose(binFile);
    return root;
}

// Show AVL - Test
void showAVL(struct TreeNode *root)
{
    if (root != NULL)
    {
        showAVL(root->left);
        printf("Category: %s\n", root->category);
        struct RecordNumberNode *recordNode = root->recordNumbers;
        while (recordNode != NULL)
        {
            printf("  Number: %d\n", recordNode->number);
            recordNode = recordNode->next;
        }
        showAVL(root->right);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------
// Indice baseado no campo "Rating", em memória
// Cria um novo nodo de cabeçalho
struct RatingIndexHashNode *newHashNode(int hash, struct RatingIndexHashNode *next)
{
    struct RatingIndexHashNode *new = (struct RatingIndexHashNode *)malloc(sizeof(struct RatingIndexHashNode));
    new->hash = hash;
    new->next = next;
    new->count = 0;
    new->firts = NULL;

    return new;
}
// Cria um novo nodo para o registro
struct RatingIndexNode *newRatingNode(int number, float rating, struct RatingIndexNode *last)
{
    struct RatingIndexNode *new = (struct RatingIndexNode *)malloc(sizeof(struct RatingIndexNode));
    new->number = number;
    new->rating = rating;
    new->last = last;
    new->next = NULL;

    return new;
}
// Gera o HASH
int getHash(struct Record record)
{
    return record.rating * 10;
}
// Cria o índice de Rating em memória
void createIndexMemoryByRating(const char *binFileName)
{
    FILE *binFile = fopen(binFileName, "rb");
    if (!binFile)
    {
        printf("Error opening the binary file.\n");
        return;
    }

    struct Record record;

    int i = 0; /*DEBUG*/

    while (
        i < LIMIT && /*DEBUG*/
        fread(&record, sizeof(struct Record), 1, binFile))
    {

        i++; /*DEBUG*/

        int hash = getHash(record);

        if (hashIndex == NULL)
        {
            hashIndex = newHashNode(hash, NULL);
            hashIndex->firts = newRatingNode(record.number, record.rating, NULL);
            hashIndex->count = 1;
        }
        else
        {
            struct RatingIndexHashNode *cur = hashIndex;
            while (cur->next != NULL && cur->hash < hash && (cur->next)->hash < hash)
                cur = cur->next;

            if (cur->next != NULL && (cur->next)->hash == hash)
                cur = cur->next;

            if (cur->hash == hash)
            {
                struct RatingIndexNode *curApp = cur->firts;
                cur->count++;

                while (curApp->next != NULL)
                    curApp = curApp->next;

                curApp->next = newRatingNode(record.number, record.rating, curApp);
            }
            else
            {
                cur->next = newHashNode(hash, cur->next);
                (cur->next)->firts = newRatingNode(record.number, record.rating, NULL);
                (cur->next)->count = 1;
            }
        }
    }

    fclose(binFile);

    printf("Hash Index created successfully.\n");
}
void printIndexMemoryByRating()
{
    struct RatingIndexHashNode *curHash = hashIndex;
    while (curHash != NULL)
    {
        printf("\nHash: %d - Qtd: %d\n", curHash->hash, curHash->count);

        struct RatingIndexNode *cur = curHash->firts;
        while (cur != NULL)
        {
            printf("%d (%.1f) | ", cur->number, cur->rating);

            cur = cur->next;
        }

        curHash = curHash->next;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------
// Arquivo de indice baseado no campo "Nome"
// Cria um novo nodo para o registro
struct IndexName *newIndexNameStruct(char *name, long offSet)
{
    struct IndexName *new = (struct IndexName *)malloc(sizeof(struct IndexName));
    strcpy(new->name, name);
    fillWithSpaces(new->name, NAME);
    new->offset = offSet;

    return new;
}
// Cria o índice de Name em disco
void createIndexFileByName(const char *binFileName, const char *indexFileName)
{
    FILE *binFile = fopen(binFileName, "rb");
    if (!binFile)
    {
        printf("Error opening the binary file.\n");
        return;
    }

    FILE *indexFile = fopen(indexFileName, "wb");
    if (!indexFile)
    {
        printf("Error creating the index file.\n");
        fclose(binFile);
        return;
    }

    int i, j, x;

    x = 0;

    for (i = 65; i <= 91; i++)
    {

        struct Record record;
        long binOffset = 0;

        j = 0; /*DEBUG*/

        fseek(binFile, 0, SEEK_SET);
        struct IndexNameList *indexList = NULL;

        while (
            // j<=LIMIT &&  /*DEBUG*/
            fread(&record, sizeof(struct Record), 1, binFile))
        {

            j++; /*DEBUG*/

            if (record.name[0] == i || (i == 91 && (record.name[0] < 65 || record.name[0] > 90)))
            {
                struct IndexName *index = newIndexNameStruct(record.name, binOffset);

                if (indexList == NULL)
                {
                    indexList = (struct IndexNameList *)malloc(sizeof(struct IndexNameList));
                    indexList->ix = index;
                    indexList->next = NULL;
                }
                else
                {
                    struct IndexNameList *cur = indexList;
                    struct IndexNameList *ant = NULL;
                    while (cur->next != NULL && strcmp((cur->ix)->name, index->name) <= 0)
                    {
                        ant = cur;
                        cur = cur->next;
                    }

                    x++;

                    struct IndexNameList *new = (struct IndexNameList *)malloc(sizeof(struct IndexNameList));
                    new->ix = index;

                    if (cur->next == NULL)
                    {
                        new->next = NULL;
                        cur->next = new;
                    }
                    else
                    {
                        if (ant != NULL)
                        {
                            ant->next = new;
                            new->next = cur;
                        }
                        else
                        {
                            new->next = cur;
                            indexList = new;
                        }
                    }
                }
            }

            binOffset += sizeof(struct Record);
        }

        while (indexList != NULL)
        {
            fwrite(indexList->ix, sizeof(struct IndexName), 1, indexFile);
            indexList = indexList->next;
            // x++;
        }
    }

    fclose(binFile);
    fclose(indexFile);

    printf("Index created successfully. Registers: %ld.\n", x);
}
void printIndexFileByName(const char *indexFileName)
{
    FILE *indexFile = fopen(indexFileName, "rb");
    if (!indexFile)
    {
        printf("Error opening the index file.\n");
        return;
    }

    struct IndexName index;
    int i = 0;

    while (fread(&index, sizeof(struct IndexName), 1, indexFile))
    {
        printf("Name: %s | OffSet: %ld\n", index.name, index.offset);
        i++;
    }

    printf("Registers: %d", i);

    fclose(indexFile);
}
//---------------------------------------------------------------------------------------------------------------------------------------------
// Case 1
void funcCase1(struct TreeNode *rootAVL, const char *indexNumberFile, const char *binFile)
{
    char searchedCategory[CATEGORY] = "Education";
    fillWithSpaces(searchedCategory, CATEGORY);
    struct RecordNumberNode *categoryNumbers = searchCategory(rootAVL, searchedCategory);
    if (categoryNumbers)
    {
        printf("Games in the '%s' category:\n", searchedCategory);
        struct RecordNumberNode *current = categoryNumbers;
        while (current != NULL)
        {
            int targetNumber = current->number;

            // Consultar o arquivo de índice para obter a posição do registro
            long idxOffset = searchByNumber(indexNumberFile, targetNumber, 2);

            if (idxOffset != -1)
            {
                FILE *binaryFile = fopen(binFile, "rb");
                if (!binaryFile)
                {
                    printf("Error opening the binary file.\n");
                    break;
                }

                fseek(binaryFile, idxOffset, SEEK_SET);
                struct Record record;
                if (fread(&record, sizeof(struct Record), 1, binaryFile))
                {
                    printf("Number: %d - Name: %s\n", record.number, record.name);
                }

                fclose(binaryFile);
            }
            else
            {
                printf("Record with number %d not found in the index.\n", targetNumber);
            }

            current = current->next;
        }
    }
    else
    {
        printf("Category '%s' not found.\n", searchedCategory);
    }
}

// Case 2
void funcCase2(const char *indexNumberFile, const char *binFile, int targetNumber)
{

    long idxOffset = searchByNumber(indexNumberFile, targetNumber, 2);

    if (idxOffset != -1)
    {
        FILE *binaryFile = fopen(binFile, "rb");
        if (!binaryFile)
        {
            printf("Error opening the binary file.\n");
        }

        fseek(binaryFile, idxOffset, SEEK_SET);
        struct Record record;
        if (fread(&record, sizeof(struct Record), 1, binaryFile))
        {
            printf("Infos:\n\nNumber: %d\nName: %s\nCategory: %s\nRating: %f\nRating count: %d\nInstalls: %s\nFree: %s\nSize: %s\nLast updated: %s\nContent rating: %s\n\n\n", record.number, record.name, record.category, record.rating, record.rating_count, record.installs, record.free, record.size, record.last_updated, record.content_rating);
        }

        fclose(binaryFile);
    }
    else
    {
        printf("Record with number %d not found in the index.\n", targetNumber);
    }
}

// Case 3
void funcCase3(const char *binFile, float rate)
{
    struct RatingIndexHashNode *curHash = hashIndex;

    printf("App's: ");
    while (curHash != NULL)
    {
        if (curHash->hash > (rate * 10))
        {
            struct RatingIndexNode *cur = curHash->firts;

            while (cur != NULL)
            {
                printf("App number: %d - Rate: %.1f\n", cur->number, cur->rating);
                cur = cur->next;
            }
        }

        curHash = curHash->next;
    }
}

// Case 4
void funcCase4(const char *indexNameFile, const char *binFile, char *name)
{
    long idxOffset = searchByName(indexNameFile, name);

    if (idxOffset != -1)
    {
        FILE *binaryFile = fopen(binFile, "rb");
        if (!binaryFile)
        {
            printf("Error opening the binary file.\n");
        }

        fseek(binaryFile, idxOffset, SEEK_SET);
        struct Record record;
        if (fread(&record, sizeof(struct Record), 1, binaryFile))
        {
            printf("Infos:\n\nNumber: %d\nName: %s\nCategory: %s\nRating: %f\nRating count: %d\nInstalls: %s\nFree: %s\nSize: %s\nLast updated: %s\nContent rating: %s\n\n\n", record.number, record.name, record.category, record.rating, record.rating_count, record.installs, record.free, record.size, record.last_updated, record.content_rating);
        }

        fclose(binaryFile);
    }
    else
    {
        printf("Record with number %s not found in the index.\n", name);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------
void showMenu(struct TreeNode *rootAVL, const char *textFile, const char *binaryFile, const char *indexNumberFile, const char *indexNameFile)
{
    int choice = 0;

    do
    {
        printf("1. What are the apps into 'Education' category?\n2. What are the informations about an specific app by name?\n3. What are the numbers of apps rated higher than X?\n4. What are the informations about an specific app by name\n\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            funcCase1(rootAVL, indexNumberFile, binaryFile);
            break;
        case 2:
            printf("What is the app number?\n");

            int number;
            scanf("%d", &number);
            funcCase2(indexNumberFile, binaryFile, number);
            break;
        case 3:
            printf("What is the app rate?\n");

            float rate;
            scanf("%f", &rate);
            funcCase3(binaryFile, rate);
            break;
        case 4:
            printf("What is the app name?\n");

            char name[NAME];
            gets(name);
            gets(name);
            funcCase4(indexNameFile, binaryFile, name);
            break;
        default:
            choice = 0;
            printf("Invalid choice");
        }
    } while (choice != 0);
}

//---------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error: <Generate .bin files (0/1)>");
        exit(0);
    }

    const char *textFile = "apps.csv";
    const char *binaryFile = "apps.bin";
    const char *indexNumberFile = "idxnumber.bin";
    const char *indexNameFile = "namesIndex.bin";

    if (atoi(argv[1]) == 1)
    {
        // Criação arquivo binário
        createBinaryFile(textFile, binaryFile);

        // Criação do arquivo sequencial indexado campo numero
        createIndexFile(binaryFile, indexNumberFile);
        // showIndexFile(indexNumberFile);

        // Criação do arquivo indexado campo Name
        createIndexFileByName(binaryFile, indexNameFile);
        // printIndexFileByName();
    }

    // Criação da AVL
    struct TreeNode *root = createAVL(binaryFile);
    // showAVL(root);

    // Criação da Hash Table
    createIndexMemoryByRating(binaryFile);

    // Mostrar menu
    showMenu(root, textFile, binaryFile, indexNumberFile, indexNameFile);

    return 0;
}