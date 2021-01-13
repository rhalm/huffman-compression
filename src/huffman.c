#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int freq, ascii_code;
    char* huffman_code;
    struct Node *left, *right;
} Node;

void selection_sort(Node* forest[], int length) {
    int minind, i, j;
    for (i = 0; i < length - 1; i++) {
        minind = i;
        /* min search in subarray */
        for (j = i + 1; j < length; j++) {
            if (forest[j]->freq < forest[minind]->freq)
                minind = j;
        }
        /* swap min - current */
        Node* temp = forest[minind];
        forest[minind] = forest[i];
        forest[i] = temp;
    }
}

Node** create_forest(int* char_freq, int length) {
    Node** forest = (Node**) malloc(sizeof(Node) * length);
    if (forest == NULL){
        printf("Memory allocation failed!");
        exit(-1);
    }
    int j = 0;
    for (int i = 0; i < 256; i++) {
        if (char_freq[i] != 0) {
            Node* new_forest = (Node*) malloc(sizeof(Node));
            if (new_forest == NULL){
                printf("Memory allocation failed!");
                exit(-1);
            }
            new_forest->freq = char_freq[i];
            new_forest->ascii_code = i;
            new_forest->left = NULL;
            new_forest->right = NULL;
            new_forest->huffman_code = (char*) malloc(sizeof(char));
            if (new_forest->huffman_code == NULL){
                printf("Memory allocation failed!");
                exit(-1);
            }
            new_forest->huffman_code[0] = '\0';
            forest[j] = new_forest;
            j++;
        }
    }
    selection_sort(forest, length);
    return forest;
}

Node** copy_forest(Node* forest[], int length) {
    Node** new_forest = (Node**) malloc(sizeof(Node) * length);
    if (new_forest == NULL){
        printf("Memory allocation failed!");
        exit(-1);
    }
    for (int i = 0; i < length; i++)
        new_forest[i] = forest[i];
    return new_forest;
}

Node* merge_two_min_nodes(Node* forest[]) {
    Node* left = forest[0];
    Node* right = forest[1];

    Node* new_forest = (Node*) malloc(sizeof(Node));
    if (new_forest == NULL){
        printf("Memory allocation failed: merge_two_min_nodes");
        exit(-1);
    }
    new_forest->freq = left->freq + right->freq;
    new_forest->left = left;
    new_forest->right = right;
    return new_forest;
}

Node** delete_two_min_nodes(Node* forest[], int* length) {
    Node** new_forest = (Node**) malloc(sizeof(Node) * (*length - 2));
    if(new_forest == NULL) {
        printf("Memory allocation failed!");
        exit(-1);
    }
    for (int i = 0; i < *length - 2; i++)
        new_forest[i] = forest[i + 2];

    *length -= 2;
    free(forest);
    return new_forest;
}

Node** add_node_to_forest(Node* forest[], Node* node, int* length) {
    Node** new_forest = (Node**) malloc(sizeof(Node) * (*length + 1));
    if(new_forest == NULL) {
        printf("Memory allocation failed!");
        exit(-1);
    }
    /* find the node's place in forest */
    int i = 0;
    while(*length > 0 && node->freq > forest[i]->freq) {
        i++;
        if (i == *length) break;
    }
    int j;
    for (j = 0; j < i; j++)
        new_forest[j] = forest[j];

    /* add node to new_forest */
    new_forest[j] = node;

    for (j = j + 1; j < *length + 1; j++)
        new_forest[j] = forest[j - 1];

    free(forest);
    (*length)++;

    return new_forest;
}

Node* build_tree(Node* forest[], int length) {
    Node* root_node;
    while (length > 1) {
        root_node = merge_two_min_nodes(forest);
        forest = delete_two_min_nodes(forest, &length);
        forest = add_node_to_forest(forest, root_node, &length);
    }
    free(forest);
    return root_node;
}

char* add_char_to_beginning(char added, char* original) {
    char* new_forest = (char*) malloc (sizeof(char) * strlen(original) + 1 + 1);
    if (new_forest == NULL){
        printf("Memory allocation failed!");
        exit(-1);
    }
    new_forest[0] = added;
    strcpy(&new_forest[1], original);
    free(original);

    return new_forest;
}

Node* traverse(Node* root_node, Node* node) {
    if (root_node == NULL)
        return NULL;

    if (root_node == node)
        return root_node;

    /* left subtree traversal */
    Node* left_node = traverse(root_node->left, node);
    if (left_node != NULL) {
        node->huffman_code = add_char_to_beginning('0', node->huffman_code);
        return left_node;
    }

    /* right subtree traversal */
    Node* right_node = traverse(root_node->right, node);
    if (right_node != NULL) {
        node->huffman_code = add_char_to_beginning('1', node->huffman_code);
        return right_node;
    }

    return NULL; // couldn't be found
}

void free_root(Node *root_node) {
   if (root_node == NULL)
      return;

   free_root(root_node->left);
   free_root(root_node->right);
   free(root_node);
}

void huffman(int* char_freq, char** codetable) {
    int nonzero_chars_count = 0;
    for (int i = 0; i < 256; i++) {
        if (char_freq[i] != 0) nonzero_chars_count++;
    }
    Node** forest = create_forest(char_freq, nonzero_chars_count);
    Node** characters_copy = copy_forest(forest, nonzero_chars_count);

    Node* root_node = build_tree(forest, nonzero_chars_count);
	
    /* make binary codetable */
    for (int i = 0; i < nonzero_chars_count; i++) {
        traverse(root_node, characters_copy[i]);
        codetable[characters_copy[i]->ascii_code] = characters_copy[i]->huffman_code;
    }

    free(characters_copy);
    free_root(root_node);
    return;
}
