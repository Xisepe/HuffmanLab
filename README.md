# HuffmanLab
This is my passed lab work for NSU
# Description
It is text compressor based on Huffman algorithm. There are implementation of Node and MinHeap structers which are essential to build Huffman tree.

![image](https://user-images.githubusercontent.com/58903139/157651374-36c1807c-bbbc-4e07-a3ab-8985161d7389.png)

Steps to build Huffman Tree
Input is an array of unique characters along with their frequency of occurrences and output is Huffman Tree. 

# Create Tree
1)Create a leaf node for each unique character and build a min heap of all leaf nodes (Min Heap is used as a priority queue. The value of frequency field is used to compare two nodes in min heap. Initially, the least frequent character is at root)
2)Extract two nodes with the minimum frequency from the min heap.
3)Create a new internal node with a frequency equal to the sum of the two nodes frequencies. Make the first extracted node as its left child and the other extracted node as its right child. Add this node to the min heap.
4)Repeat steps#2 and #3 until the heap contains only one node. The remaining node is the root node and the tree is complete.

# Encode
To encode file we need to build codes for each symbol.

1)If you go to left node, write 0 bit
2)If you go to right node, write 1 bit

Now each symbol from source can be represented as bit sequence and writed in destination file.

To decode char sequence we need to know how it was encdoed. So we encode Huffmun tree, NOT SYMBOL FREQUENCIES because it can costs a lot of memory and there will be no use in compressing. 

So for each node, starting at root:

1)If leaf-node: Output 1-bit + N-bit character/byte
2)If not leaf-node, output 0-bit. Then encode both child nodes (left first then right) the same way

# Decode
Steps to decode tree :
1)Read bit. 
2)If 1, then read N-bit character/byte, return new node around it with no children
3)If bit was 0, decode left and right child-nodes the same way, and return new node around them with those children, but no value

Steps to decode text:
1)Read bit
2)If it 1-bit go to right node
3)If it 0-bit go to left node
4)Do previous steps until you reach leaf node. It will be decoded symbol

