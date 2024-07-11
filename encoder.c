#include <stdio.h>
#include <stdlib.h>


typedef struct CharacterCount
{
    char character;
    unsigned int count;
} CharacterCount;

typedef struct CharacterCountTree
{
    CharacterCount *data;
    struct CharacterCountTree *left;
    struct CharacterCountTree *right;
} CharacterCountTree;

typedef struct CharacterCountNode
{
    CharacterCountTree *data;
    struct CharacterCountNode *next;
} CharacterCountNode;


int compare_character_count(const void *elem1, const void *elem2)
{
    CharacterCount *char_count_1 = (CharacterCount*) elem1;
    CharacterCount *char_count_2 = (CharacterCount*) elem2;
    return (char_count_1->count - char_count_2->count);
}

int count_characters(char *filename, CharacterCount *summary)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }
    
    char ch;
    char char_count[128] = {0};
    while ((ch = fgetc(file)) != EOF)
    {
        char_count[(int) ch]++;
    }
    
    int num_different_chars = 0;
    for (int i = 0; i < 128; i++)
    {
        if (char_count[i])
        {
            summary[num_different_chars].character = i;
            summary[num_different_chars].count = char_count[i];
            num_different_chars++;
        }
    }

    qsort(summary, num_different_chars, sizeof(CharacterCount), compare_character_count);

    return num_different_chars;
}


CharacterCountNode *create_list(CharacterCount *summary, int num_different_chars)
{
    CharacterCountNode *first = malloc(sizeof(CharacterCountNode));
    CharacterCountTree *tree = malloc(sizeof(CharacterCountTree));
    first->data = tree;
    first->next = NULL;
    tree->data = &summary[0];
    tree->left = NULL;
    tree->right = NULL;
    CharacterCountNode *prev = first;
    for (int i = 0; i < num_different_chars; i++)
    {
        CharacterCountNode *node = malloc(sizeof(CharacterCountNode));
        CharacterCountTree *tree = malloc(sizeof(CharacterCountTree));
        node->data = tree;
        node->next = NULL;
        tree->data = &summary[i];
        tree->left = NULL;
        tree->right = NULL;
        prev = node;
    }
    return first;
}


CharacterCount *create_character_count(char character, unsigned int count)
{
    CharacterCount *character_count = malloc(sizeof(CharacterCount));
    character_count->character = character;
    character_count->count = count;
    return character_count;
}


CharacterCountTree *build_huffman_tree(CharacterCountNode *character_count_list)
{
    if (character_count_list->next == NULL)
    {
        // Only one element in the list
        return character_count_list->data;
    }

    // Create the new tree with the first two elements
    CharacterCountTree *tree = malloc(sizeof(CharacterCountTree));
    tree->left = character_count_list->data;
    tree->right = character_count_list->next->data;
    tree->data = create_character_count(0, tree->left->data->count + tree->right->data->count);
    CharacterCountNode *new_node = malloc(sizeof(CharacterCountNode));
    new_node->data = tree;

    // Find the first element in the list with a count greater
    CharacterCountNode *node = character_count_list->next->next;
    CharacterCountNode *prev = character_count_list->next;
    while (node && tree->data->count > node->data->data->count)
    {
        prev = node;
        node = node->next;
    }

    // Insert the new node in the list
    prev->next = new_node;
    new_node->next = node;
    
    // Remove the two first elements of the list, modify the starting point of the
    // list and call the function recursively
    CharacterCountNode *temp = character_count_list->next;
    free(character_count_list);
    character_count_list = temp->next;
    free(temp);
    return build_huffman_tree(character_count_list);
}


void print_huffman_tree(CharacterCountTree *huffman_tree)
{
    if (huffman_tree->left == NULL && huffman_tree->right == NULL)
    {
        printf("char: %c, count: %u", huffman_tree->data->character, huffman_tree->data->count);
        return;
    }
    print_huffman_tree(huffman_tree->left);
    print_huffman_tree(huffman_tree->right);
}


void encode(char *filename)
{
    CharacterCount summary[128] = {0};
    int num_different_chars = count_characters(filename, summary);
    if (num_different_chars == 0)
    {
        printf("Arquivo vazio!\n");
        exit(1);
    }
    CharacterCountNode *character_count_list = create_list(summary, num_different_chars);
    CharacterCountTree *huffman_tree = build_huffman_tree(character_count_list);
    print_huffman_tree(huffman_tree);
}


void print_help()
{
    printf("Para usar o compactador rode:\n");
    printf("\t./compacta nome_arquivo.txt\n");
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        print_help();
        exit(1);
    }

    encode(argv[1]);

    return 0;
}