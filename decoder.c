#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct huffman_decode
{
    char character;
    struct huffman_decode *left;
    struct huffman_decode *right;
} HuffmanDecode;

FILE *open_file_read(char *filename)
{
    FILE *in = fopen(filename, "rb");
    if (in == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }
    return in;
}

FILE *open_file_write(char *filename)
{
    FILE *out = fopen(filename, "w");
    if (out == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }
    return out;
}

uint8_t get_number_of_different_chars(FILE *in, uint64_t *total_chars)
{
    fread(total_chars, sizeof(uint64_t), 1, in);
    uint8_t num_different_chars = 0;
    fread(&num_different_chars, 1, 1, in);
    return num_different_chars;
}

void insert_new_character(HuffmanDecode *huffman, char character, uint8_t code_length, uint8_t *code)
{
    for (int j = 0; j < code_length / 8; j++)
    {
        for (int k = 7; k >= 0; k--)
        {
            if (code[j] & (1 << k))
            {
                if (huffman->right == NULL)
                {
                    huffman->right = malloc(sizeof(HuffmanDecode));
                    huffman->right->character = 0;
                    huffman->right->left = NULL;
                    huffman->right->right = NULL;
                }
                huffman = huffman->right;
            }
            else
            {
                if (huffman->left == NULL)
                {
                    huffman->left = malloc(sizeof(HuffmanDecode));
                    huffman->left->character = 0;
                    huffman->left->left = NULL;
                    huffman->left->right = NULL;
                }
                huffman = huffman->left;
            }
        }
    }
    for (int k = code_length % 8 - 1; k >= 0; k--)
    {
        if (code[code_length/8] & (1 << k))
        {
            if (huffman->right == NULL)
            {
                HuffmanDecode *new_huffman = malloc(sizeof(HuffmanDecode));
                new_huffman->character = 0;
                new_huffman->left = NULL;
                new_huffman->right = NULL;
                huffman->right = new_huffman;
            }
            huffman = huffman->right;
        }
        else
        {
            if (huffman->left == NULL)
            {
                HuffmanDecode *new_huffman = malloc(sizeof(HuffmanDecode));
                new_huffman->character = 0;
                new_huffman->left = NULL;
                new_huffman->right = NULL;
                huffman->left = new_huffman;
            }
            huffman = huffman->left;
        }
    }
    huffman->character = character;
}

HuffmanDecode *read_header(FILE *in, uint8_t num_different_chars)
{
    HuffmanDecode *huffman = malloc(sizeof(HuffmanDecode));
    huffman->character = 0;
    huffman->left = NULL;
    huffman->right = NULL;
    for (uint8_t i = 0; i < num_different_chars; i++)
    {
        char character;
        fread(&character, 1, 1, in);
        uint8_t code_length;
        fread(&code_length, 1, 1, in);
        uint8_t *code = malloc((code_length-1)/8 + 1);
        fread(code, 1, (code_length-1)/8 + 1, in);
        insert_new_character(huffman, character, code_length, code);
    }
    return huffman;
}

void decode(FILE *in, FILE *out, HuffmanDecode *huffman, uint64_t total_chars)
{
    uint8_t buffer;
    uint64_t chars_written = 0;
    HuffmanDecode *current_state = huffman;
    while (fread(&buffer, 1, 1, in) > 0)
    {
        for (int i = 7; i >= 0; i--)
        {
            if (buffer & (1 << i))
            {
                current_state = current_state->right;
            }
            else
            {
                current_state = current_state->left;
            }
            if (current_state == NULL)
            {
                return;
            }
            if (current_state->character != 0)
            {
                fputc(current_state->character, out);
                chars_written++;
                if (chars_written == total_chars)
                {
                    return;
                }
                current_state = huffman;
            }
        }
    }
}

void print_help()
{
    printf("Para usar o descompactador rode:\n");
    printf("\t./descompacta nome_arquivo.comp\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        print_help();
        exit(1);
    }

    char *filename = argv[1];

    // Abrir arquivo em modo de leitura binario
    FILE *in = open_file_read(filename);
    char output_filename[100];
    strcpy(output_filename, filename);
    strcat(output_filename, ".txt");
    FILE *out = open_file_write(output_filename);

    // Ler cabeçalho e guardar informações da codificação
    uint64_t total_chars;
    uint8_t num_different_chars = get_number_of_different_chars(in, &total_chars);
    HuffmanDecode *huffman = read_header(in, num_different_chars);

    // Gerar arquivo descompactado
    decode(in, out, huffman, total_chars);

    fclose(in);
    fclose(out);

    return 0;
}
