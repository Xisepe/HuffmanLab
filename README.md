# HuffmanLab
This is my passed lab work for NSU
# Description
It is text compressor based on Huffman algorithm. There are implementation of Node and MinHeap structers which are essential to build Huffman tree.

![image](https://user-images.githubusercontent.com/58903139/157651374-36c1807c-bbbc-4e07-a3ab-8985161d7389.png)

Steps to build Huffman Tree
Input is an array of unique characters along with their frequency of occurrences and output is Huffman Tree. 

Create a leaf node for each unique character and build a min heap of all leaf nodes (Min Heap is used as a priority queue. The value of frequency field is used to compare two nodes in min heap. Initially, the least frequent character is at root)
Extract two nodes with the minimum frequency from the min heap.
 
Create a new internal node with a frequency equal to the sum of the two nodes frequencies. Make the first extracted node as its left child and the other extracted node as its right child. Add this node to the min heap.
Repeat steps#2 and #3 until the heap contains only one node. The remaining node is the root node and the tree is complete.

To decode char sequence we need to know how it was encdoed. So there encoded Huffmun tree, NOT SYMBOL FREQUENCIES becouse it can costs a lot of memory and there will be no use in compressing. 

So for each node, starting at root:

If leaf-node: Output 1-bit + N-bit character/byte
If not leaf-node, output 0-bit. Then encode both child nodes (left first then right) the same way
To read, do this:

Read bit. If 1, then read N-bit character/byte, return new node around it with no children
If bit was 0, decode left and right child-nodes the same way, and return new node around them with those children, but no value
