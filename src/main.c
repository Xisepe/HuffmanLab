#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "consts.h"

uint8_t g_byte;
uint8_t g_mask = 1 << 7;
size_t g_index = 0;

typedef struct Node {
    uint8_t val;
    uint32_t weight;

    struct Node *left;
    struct Node *right;

} Node;

void freeTree(Node *p_root) {
    if (!p_root) {
        return;
    }
    freeTree(p_root->left);
    freeTree(p_root->right);
    free(p_root);
}

size_t getParent(const size_t index) {
    if ((int64_t) index - 1 < 0) {
        return SIZE_MAX;
    }
    return (index - 1) / 2;
}

uint32_t getWeight(const Node *p_node) {
    return p_node->weight;
}

char isLeaf(const Node *p_node) {
    if (!p_node->right && !p_node->left)
        return 1;
    else
        return 0;
}

Node *createNode(const uint8_t val, const uint32_t weight, Node *p_left, Node *p_right) {
    Node *res = (Node *) malloc(sizeof(Node));
    res->val = val;
    res->weight = weight;
    res->left = p_left;
    res->right = p_right;
    return res;
}

void swapNodes(Node *p_first, Node *p_second) {
    Node tmp = *p_first;
    *p_first = *p_second;
    *p_second = tmp;
}

typedef struct MinHeap {
    size_t size;
    size_t capacity;

    Node **nodes;
} MinHeap;

MinHeap *createMinHeap(const size_t capacity) {
    MinHeap *p_minHeap = (MinHeap *) malloc(sizeof(MinHeap));
    p_minHeap->capacity = capacity;
    p_minHeap->size = 0;
    p_minHeap->nodes = (Node **) malloc(capacity * sizeof(Node *));
    return p_minHeap;
}

void minHeapify(MinHeap *p_minHeap, const size_t index) {
    size_t smallest = index;
    size_t left = index * 2 + 1;
    size_t right = index * 2 + 2;

    if (left < p_minHeap->size && getWeight(p_minHeap->nodes[left]) < getWeight(p_minHeap->nodes[smallest]))
        smallest = left;
    if (right < p_minHeap->size && getWeight(p_minHeap->nodes[right]) < getWeight(p_minHeap->nodes[smallest]))
        smallest = right;

    if (smallest != index) {
        swapNodes(p_minHeap->nodes[smallest], p_minHeap->nodes[index]);
        minHeapify(p_minHeap, smallest);
    }
}

Node *pop(MinHeap *p_minHeap) {
    swapNodes(p_minHeap->nodes[0], p_minHeap->nodes[p_minHeap->size - 1]);
    p_minHeap->size--;
    minHeapify(p_minHeap, 0);
    return p_minHeap->nodes[p_minHeap->size];
}

Node *peek(MinHeap *p_minHeap) {
    return p_minHeap->nodes[0];
}

void insert(MinHeap *p_minHeap, Node *p_node) {
    p_minHeap->size++;
    size_t i;
    for (i = p_minHeap->size - 1; i && p_node->weight < p_minHeap->nodes[getParent(i)]->weight; i = getParent(i)) {
        p_minHeap->nodes[i] = p_minHeap->nodes[getParent(i)];
    }
    p_minHeap->nodes[i] = p_node;
}

void buildMinHeap(MinHeap *p_minHeap) {
    for (size_t i = getParent(p_minHeap->size - 1); i != SIZE_MAX; i--) {
        minHeapify(p_minHeap, i);
    }
}

int getNumberOfNonZero(const uint32_t *a_buffer, const size_t size) {
    int c = 0;
    for (size_t i = 0; i < size; ++i) {
        if (a_buffer[i] != 0)
            c++;
    }
    return c;
}

MinHeap *buildMinHeapFromData(const uint32_t a_weights[], const size_t weightSize) {
    MinHeap *p_heap = createMinHeap(getNumberOfNonZero(a_weights, weightSize));
    for (size_t i = 0; i < weightSize; ++i) {
        if (a_weights[i] != 0) {
            p_heap->size++;
            p_heap->nodes[p_heap->size - 1] = createNode(i, a_weights[i], NULL, NULL);
        }
    }
    buildMinHeap(p_heap);
    return p_heap;
}

void clearHeap(MinHeap *p_heap) {
    free(p_heap->nodes);
    free(p_heap);
}

Node *createHuffmanTree(const uint32_t a_weights[], const size_t weightSize) {
    MinHeap *p_heap = buildMinHeapFromData(a_weights, weightSize);

    while (p_heap->size != 1) {
        Node *p_left = pop(p_heap);
        Node *p_right = pop(p_heap);

        Node *root = createNode(0, getWeight(p_left) + getWeight(p_right), p_left, p_right);

        insert(p_heap, root);
        p_left = NULL;
        p_right = NULL;
    }
    Node *root = peek(p_heap);
    clearHeap(p_heap);
    return root;
}

void analyzeInput(const uint8_t *a_buffer, const size_t size, uint32_t *destination) {
    for (size_t i = 0; i < size; ++i) {
        destination[(size_t) a_buffer[i]]++;
    }
}

uint32_t *readAndAnalyze(FILE *source, const size_t alphabetSize) {
    uint8_t buffer[BUFFER_SIZE];
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = 0;
    }
    uint32_t *result = (uint32_t *) calloc(alphabetSize, sizeof(uint32_t));
    size_t read = fread(buffer, sizeof(uint8_t), BUFFER_SIZE, source);
    if (read == 0) {
        free(result);
        return NULL;
    }
    do {
        analyzeInput(buffer, read, result);
    } while ((read = fread(buffer, sizeof(uint8_t), BUFFER_SIZE, source)));
    return result;
}

uint32_t *getWeightMap(FILE *stream, const size_t alphabetSize) {
    return readAndAnalyze(stream, alphabetSize);
}

typedef struct Code {
    unsigned long code;
    unsigned long code_mask;
} Code;

Code createCode(const unsigned long code, const unsigned long mask) {
    return (Code) {code, mask};
}

unsigned long writeInLong(unsigned long code, const long val) {
    return (val & 1) ? ((code << 1) | 1) : (code << 1);
}

void createCodes(const Node *p_root, Code *a_codes, const unsigned long code, const unsigned long mask) {
    if (p_root->left) {
        unsigned long tmp_code = writeInLong(code, 0ul);
        unsigned long tmp_mask = mask;
        if (tmp_mask == 0ul) tmp_mask = 1;
        else tmp_mask <<= 1ul;
        createCodes(p_root->left, a_codes, tmp_code, tmp_mask);
    }
    if (p_root->right) {
        unsigned long tmp_code = writeInLong(code, 1ul);
        unsigned long tmp_mask = mask;
        if (tmp_mask == 0ul) tmp_mask = 1ul;
        else tmp_mask <<= 1ul;
        createCodes(p_root->right, a_codes, tmp_code, tmp_mask);
    }
    if (isLeaf(p_root)) {
        if (code == 0ul && mask == 0ul) {
            a_codes[(size_t) p_root->val] = createCode(0ul, 1ul);
        } else
            a_codes[(size_t) p_root->val] = createCode(code, mask);
    }
}

Code *getCodesMap(const Node *p_root, const size_t size) {
    Code *result = (Code *) malloc(size * sizeof(Code));
    for (size_t i = 0; i < size; ++i) {
        result[i] = createCode(0ul, 0ul);
    }
    createCodes(p_root, result, 0ul, 0ul);
    return result;
}

uint8_t writeBit(uint8_t byte, const uint8_t mask) {
    byte |= mask;
    return byte;
}

uint8_t readBit(const uint8_t byte, const uint8_t mask) {
    if (byte & mask) {
        return 1;
    }
    return 0;
}

uint8_t isFull(const uint8_t mask) {
    if (mask) {
        return 0;
    }
    return 1;
}

uint8_t getZeroByte() {
    return (uint8_t) 0;
}

uint8_t getNewMask() {
    return (uint8_t) (1 << 7);
}

uint8_t shiftMaskOnRight(const uint8_t mask) {
    return mask >> 1;
}

void writeInBuffer(uint8_t *a_buffer, const uint8_t byte, const size_t index) {
    a_buffer[index] = byte;
}

char isBufferOverloaded(const size_t size, const size_t index) {
    if (index < size)
        return 0;
    else
        return 1;
}

void writeBuffer(FILE *destination, const uint8_t a_buffer[], const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        fprintf(destination, "%c", a_buffer[i]);
    }
}

void initializeWithZeros(uint8_t a_buffer[], const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        a_buffer[i] = 0;
    }
}

void encodeTree(const Node *p_node, uint8_t *a_buffer) {
    if (isLeaf(p_node)) {
        if (isFull(g_mask)) {
            writeInBuffer(a_buffer, g_byte, g_index++);
            g_byte = getZeroByte();
            g_mask = getNewMask();
        }
        g_byte = writeBit(g_byte, g_mask);
        g_mask = shiftMaskOnRight(g_mask);
        const uint8_t val = p_node->val;
        uint8_t valMask = getNewMask();
        for (int i = 0; i < 8; ++i) {
            if (isFull(g_mask)) {
                writeInBuffer(a_buffer, g_byte, g_index++);
                g_byte = getZeroByte();
                g_mask = getNewMask();
            }
            if (readBit(val, valMask)) {
                g_byte = writeBit(g_byte, g_mask);
            }
            valMask = shiftMaskOnRight(valMask);
            g_mask = shiftMaskOnRight(g_mask);
        }
    } else {
        if (isFull(g_mask)) {
            writeInBuffer(a_buffer, g_byte, g_index++);
            g_byte = getZeroByte();
            g_mask = getNewMask();
        }
        g_mask = shiftMaskOnRight(g_mask);
        encodeTree(p_node->left, a_buffer);
        encodeTree(p_node->right, a_buffer);
    }
}

void encodeNumberOfCharacters(const uint32_t numberOfCharacters, uint8_t *a_buffer) {
    for (int i = 31; i >= 0; --i) {
        if (isFull(g_mask)) {
            writeInBuffer(a_buffer, g_byte, g_index++);
            g_byte = getZeroByte();
            g_mask = getNewMask();
        }
        if (numberOfCharacters & ((uint32_t) 1 << i)) {
            g_byte = writeBit(g_byte, g_mask);
        }
        g_mask = shiftMaskOnRight(g_mask);
    }
}

void encodeText(FILE *source, FILE *destination, const Code a_codes[], uint8_t a_encode[], uint8_t a_read[]) {
    size_t r;
    fseek(source, 1, SEEK_SET);
    while ((r = fread(a_read, sizeof(uint8_t), BUFFER_SIZE, source))) {
        for (size_t j = 0; j < r; ++j) {
            unsigned long code = a_codes[a_read[j]].code;
            unsigned long mask = a_codes[a_read[j]].code_mask;
            while (mask) {
                if (isBufferOverloaded(BUFFER_SIZE, g_index)) {
                    writeBuffer(destination, a_encode, BUFFER_SIZE);
                    g_index = 0;
                }
                if (isFull(g_mask)) {
                    writeInBuffer(a_encode, g_byte, g_index++);
                    g_byte = getZeroByte();
                    g_mask = getNewMask();
                }
                if (code & mask) {
                    g_byte = writeBit(g_byte, g_mask);
                }
                g_mask = shiftMaskOnRight(g_mask);
                mask >>= 1ul;
            }
        }
    }
    if (isBufferOverloaded(BUFFER_SIZE, g_index)) {
        writeBuffer(destination, a_encode, BUFFER_SIZE);
        fprintf(destination, "%c", g_byte);
    } else {
        writeInBuffer(a_encode, g_byte, g_index++);
        writeBuffer(destination, a_encode, g_index);
    }

}

void encodeAndWrite(FILE *source, FILE *destination, uint32_t *weights, const size_t numberOfCharacters) {
    if (weights == NULL)
        return;
    Node *p_root = createHuffmanTree(weights, numberOfCharacters);
    Code *p_codes = getCodesMap(p_root, numberOfCharacters);
    uint8_t encode[BUFFER_SIZE];
    uint8_t read[BUFFER_SIZE];

    initializeWithZeros(encode, BUFFER_SIZE);
    initializeWithZeros(encode, BUFFER_SIZE);

    encodeTree(p_root, encode);
    encodeNumberOfCharacters(p_root->weight, encode);
    encodeText(source, destination, p_codes, encode, read);

    freeTree(p_root);
    free(p_codes);
}

Node *decodeTree(uint8_t const a_read[]) {
    if (isFull(g_mask)) {
        g_mask = getNewMask();
        g_byte = a_read[g_index++];
    }
    if (readBit(g_byte, g_mask)) {
        g_mask = shiftMaskOnRight(g_mask);
        uint8_t val = getZeroByte();
        uint8_t val_mask = getNewMask();
        for (int i = 0; i < 8; ++i) {
            if (isFull(g_mask)) {
                g_mask = getNewMask();
                g_byte = a_read[g_index++];
            }
            if (readBit(g_byte, g_mask)) {
                val = writeBit(val, val_mask);
            }
            val_mask = shiftMaskOnRight(val_mask);
            g_mask = shiftMaskOnRight(g_mask);
        }
        return createNode(val, 0, NULL, NULL);
    } else {
        if (isFull(g_mask)) {
            g_mask = getNewMask();
            g_byte = a_read[g_index++];
        }
        g_mask = shiftMaskOnRight(g_mask);
        Node *p_left = decodeTree(a_read);
        Node *p_right = decodeTree(a_read);
        return createNode('$', 0, p_left, p_right);
    }
}

uint32_t decodeNumberOfSymbols(uint8_t const a_read[]) {
    uint32_t result = 0;
    for (int i = 31; i >= 0; --i) {
        if (isFull(g_mask)) {
            g_mask = getNewMask();
            g_byte = a_read[g_index++];
        }
        if (readBit(g_byte, g_mask)) {
            result |= ((uint32_t) 1 << i);
        }
        g_mask = shiftMaskOnRight(g_mask);
    }
    return result;
}

void
decodeText(FILE *source, FILE *destination, uint8_t a_read[], uint8_t a_decode[], uint32_t numberOfSymbols,
           Node *p_root) {
    size_t decode_index = 0;
    Node *tmp_node = p_root;
    while (numberOfSymbols) {
        char state = 1;
        while (state) {
            if (isBufferOverloaded(BUFFER_SIZE, g_index)) {
                if (fread(a_read, sizeof(uint8_t), BUFFER_SIZE, source)) {
                    g_index = 0;
                }
            }
            if (isFull(g_mask)) {
                g_mask = getNewMask();
                g_byte = a_read[g_index++];
            }
            if (!readBit(g_byte, g_mask)) {
                tmp_node = tmp_node->left;
            } else {
                tmp_node = tmp_node->right;
            }
            g_mask = shiftMaskOnRight(g_mask);
            if (tmp_node == NULL || isLeaf(tmp_node)) {
                if (isBufferOverloaded(BUFFER_SIZE, decode_index)) {
                    writeBuffer(destination, a_decode, BUFFER_SIZE);
                    decode_index = 0;
                }
                if (tmp_node == NULL) {
                    writeInBuffer(a_decode, p_root->val, decode_index++);
                } else {
                    writeInBuffer(a_decode, tmp_node->val, decode_index++);
                }
                tmp_node = p_root;
                numberOfSymbols--;
                state = 0;
            }
        }
    }
    writeBuffer(destination, a_decode, decode_index);
    tmp_node = NULL;
}

void decodeAndWrite(FILE *source, FILE *destination) {
    uint8_t a_read[BUFFER_SIZE];
    uint8_t a_decode[BUFFER_SIZE];
    initializeWithZeros(a_read, BUFFER_SIZE);
    initializeWithZeros(a_decode, BUFFER_SIZE);

    if (!fread(a_read, sizeof(uint8_t), BUFFER_SIZE, source)) {
        return;
    }
    g_byte = a_read[0];
    ++g_index;

    Node *p_root = decodeTree(a_read);
    uint32_t numberOfCharacters = decodeNumberOfSymbols(a_read);
    decodeText(source, destination, a_read, a_decode, numberOfCharacters, p_root);

    freeTree(p_root);
}


int main(void) {
    FILE *source = fopen("in.txt", "r");
    FILE *destination = fopen("out.txt", "w");

    char mode = 0;
    if (!fscanf(source, "%c", &mode)) {
        fclose(source);
        fclose(destination);
        return 0;
    }
    if (mode == 'c') {
        uint32_t *a_weights = getWeightMap(source, ALPHABET_SIZE);
        if (a_weights != NULL) {
            encodeAndWrite(source, destination, a_weights, ALPHABET_SIZE);
            free(a_weights);
        }
    } else {
        decodeAndWrite(source, destination);
    }
    fclose(source);
    fclose(destination);

    return EXIT_SUCCESS;
}

