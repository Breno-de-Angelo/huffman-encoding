#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct huffman_encode
{
    uint8_t code;
    uint8_t code_length;
} HuffmanEncode;

typedef struct character_count
{
    char ch;
    unsigned int count;
} CharacterCount;

typedef struct huffman_tree
{
    CharacterCount *info;
    struct huffman_tree *next;
    struct huffman_tree *left;
    struct huffman_tree *right;
} HuffmanTree;


int count_characters(char *filename, CharacterCount *char_count)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }

    // Contando o numero de ocorrencias
    char ch;
    while ((ch = fgetc(file)) != EOF)
    {
        char_count[(int) ch].count++;
    }
    fclose(file);

    // Garantindo que os caracteres presentes estejam nas posições iniciais
    // e escrevendo o campo de indicação do caracter
    int num_different_chars = 0;
    for (int i = 0; i < 128; i++)
    {
        if (char_count[i].count > 0)
        {
            char_count[num_different_chars].ch = (char) i;
            char_count[num_different_chars].count = char_count[i].count;
            num_different_chars++;
        }
    }

    return num_different_chars;
}

int compare_character_count(const void *elem1, const void *elem2)
{
    CharacterCount *char_count_1 = (CharacterCount*) elem1;
    CharacterCount *char_count_2 = (CharacterCount*) elem2;
    return (char_count_1->count - char_count_2->count);
}

HuffmanTree *build_huffman_tree(CharacterCount *char_count, int num_different_chars)
{
    // Inicializando a estrutura como lista encadeada
    HuffmanTree *prim = malloc(sizeof(HuffmanTree));
    prim->info = &char_count[0];
    prim->left = NULL;
    prim->right = NULL;
    prim->next = NULL;
    HuffmanTree *temp = prim;
    for (int i = 1; i < num_different_chars; i++)
    {
        HuffmanTree *huffman_tree = malloc(sizeof(HuffmanTree));
        huffman_tree->info = &char_count[i];
        huffman_tree->left = NULL;
        huffman_tree->right = NULL;
        huffman_tree->next = NULL;
        temp->next = huffman_tree;
        temp = huffman_tree;
    }
    
    // Montando árvore conforme algoritmo explicado no material disponizado pela professora
    while (prim->next != NULL)
    {
        // Criando novo nó com a soma dos dois nós menos frequentes
        CharacterCount *new_char_count = malloc(sizeof(CharacterCount));
        new_char_count->ch = 0;
        new_char_count->count = prim->info->count + prim->next->info->count;
        HuffmanTree *new_tree = malloc(sizeof(HuffmanTree));
        new_tree->left = prim;
        new_tree->right = prim->next;
        new_tree->info = new_char_count;

        // Inserindo novo nó na lista
        prim = prim->next->next;
        if (prim == NULL)
        {
            prim = new_tree;
            new_tree->next = NULL;
            continue;
        }
        HuffmanTree *temp = prim;
        HuffmanTree *prev = NULL;
        while (temp && temp->info->count < new_tree->info->count)
        {
            prev = temp;
            temp = temp->next;
        }
        if (prev == NULL)
        {
            new_tree->next = prim;
            prim = new_tree;
            continue;
        }
        new_tree->next = temp;
        prev->next = new_tree;
    }
    return prim;
}

void build_huffman_encoding_recursion(HuffmanTree *huffman_tree, HuffmanEncode *huffman, uint8_t code, int code_length)
{
    if (huffman_tree == NULL)
    {
        return;
    }
    char ch = huffman_tree->info->ch;
    if (ch != 0)
    {
        huffman[ch].code = code;
        huffman[ch].code_length = code_length;
    }
    build_huffman_encoding_recursion(huffman_tree->left, huffman, (code << 1), code_length + 1);
    build_huffman_encoding_recursion(huffman_tree->right, huffman, (code << 1 | 1), code_length + 1);
}

void build_huffman_encoding(HuffmanTree *huffman_tree, HuffmanEncode *huffman)
{
    uint8_t code = 0;
    int code_length = 1;
    if (huffman_tree->left == NULL && huffman_tree->right == NULL)
    {
        build_huffman_encoding_recursion(huffman_tree, huffman, code, code_length);
        return;
    }
    build_huffman_encoding_recursion(huffman_tree->left, huffman, code, code_length);
    code = 1;
    build_huffman_encoding_recursion(huffman_tree->right, huffman, code, code_length);
}

void print_huffman_encoding(HuffmanEncode *huffman)
{
    for (int i = 0; i < 128; i++)
    {
        if (huffman[i].code_length != 0)
        {
            printf("Char: %c, Code: ", i);
            for (int j = huffman[i].code_length - 1; j >= 0; j--)
            {
                printf("%c", (huffman[i].code & (1 << j))? '1' : '0');
            }
            printf("\n");
        }
    }
}

void write_compressed_file(char *filename, HuffmanEncode *huffman, int num_different_chars)
{
    FILE *in = fopen(filename, "r");
    if (in == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }
    char output_filename[100];
    strcpy(output_filename, filename);
    strcat(output_filename, ".comp");
    FILE *out = fopen(output_filename, "wb");
    if (out == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }

    // Primeiro será escrito um cabeçalho com as informações da codificação para permitir
    // que seja feita a decodificação posteriormente. O cabeçalho tem o seguinte formato:
    // <numero_de_caracteres_distintos><caracter><codigo><numero_de_bits_no_codigo><caracter><codigo>...
    // ...<caracter><codigo><numero_de_bits_no_codigo><texto_codificado>
    // Cada campo ocupa um byte.
    uint8_t num_different_chars_8bits = (uint8_t) num_different_chars;
    fwrite(&num_different_chars_8bits, 1, 1, out);
    for (int i = 0; i < 128; i++)
    {
        if (huffman[i].code_length > 0)
        {
            char ch = (char) i;
            fwrite(&ch, 1, 1, out);
            fwrite(&huffman[i].code, 1, 1, out);
            fwrite(&huffman[i].code_length, 1, 1, out);
        }
    }
    
    uint8_t code = 0;
    uint8_t bits_used = 0;
    char ch_read;
    while ((ch_read = fgetc(in)) != EOF)
    {
        if (huffman[ch_read].code_length + bits_used < 8)
        {
            code <<= huffman[ch_read].code_length;
            code |= huffman[ch_read].code;
            bits_used += huffman[ch_read].code_length;
        }
        else
        {
            code <<= (8 - bits_used);
            code |= (huffman[ch_read].code >> (huffman[ch_read].code_length - (8 - bits_used)));
            fwrite(&code, 1, 1, out);
            code = 0;
            code |= (huffman[ch_read].code >> (8 - bits_used));
            bits_used = huffman[ch_read].code_length - (8 - bits_used);
        }
    }
    if (bits_used != 0)
    {
        code <<= (8 - bits_used);
        fwrite(&code, 1, 1, out);
    }
    fclose(in);
    fclose(out);
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

    char *filename = argv[1];

    // Considerando um arquivo de texto ASCII, só há 128 caracteres possíveis (até menos na realidade).
    // Dessa forma, é possível construir parte das variáveis do programa na stack sem problemas
    // de escalabilidade. O i-ésimo elemnto da lista corresponde às informações da codificação do
    // caracter de ASCII = i
    HuffmanEncode huffman[128] = {0};

    // O primeiro passo é contar a ocorrência dos caracteres
    CharacterCount char_count[128] = {0};
    int num_different_chars;
    num_different_chars = count_characters(filename, char_count);
    if (num_different_chars == 0)
    {
        printf("Arquivo vazio!\n");
        exit(1);
    }

    // E ordenar o vetor com base na frequência dos caracteres. Como foi salvo o número de caracteres
    // distintos (e estes foram colocados no início do vetor), é possível ordenar apenas parte do vetor
    qsort(char_count, num_different_chars, sizeof(CharacterCount), compare_character_count);

    // Construir árvore de huffman. Para isso temos uma estrutura HuffmanTree que inicialmente se
    // comporta como lista encadeada (por meio do campo next) e aos poucos vai se tornando uma árvore
    // (por meio dos parâmetros left e right).
    HuffmanTree *huffman_tree = build_huffman_tree(char_count, num_different_chars);

    // Criar vetor de codificação
    build_huffman_encoding(huffman_tree, huffman);

    // Verificar a codificação para debug
    print_huffman_encoding(huffman);

    // Escrever arquivo usando a codificação
    write_compressed_file(filename, huffman, num_different_chars);

    return 0;
}