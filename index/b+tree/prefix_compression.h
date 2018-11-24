#pragma once

/*
creating B+-tree indices on string-valued attributes raises two problems. The
fiest problem is that strings can be of variable length. The second problem is that
strings can be long, leading to a low fanout and a correspondingly increased tree hright.

With variable-length search keys, different nodes can have different fanouts
even if they are full. A node must then be split if it is full, that is, there is no space
to add a new entry, regardless of how many search entris it has. Similarly, nodes
can be merged or entries redistributed depending on what fraction of the space
in the nodes is used, isntead of being based on the maximum number of entries
that the node can hold.

The fanout of nodes can be increased by using a technique called prefix 
compression. With prefix compression, we do not store the entire search key
value at nonleaf nodes. We only store a prefix of each search key value that is
sufficient to distinguish between the key values in the subtrees that it separates.
For example, if we had an index on names, the key value at a nonleaf node could
be a prefix of a name; it may suffice to store "Silb" at a nonleaf node, instead of
the full "silberschatz" if the closest values in the two subtrees that it separates
are, say, "Silas" and "silver" respectively.
*/

class prefix_compression
{
private:
    
public:
    prefix_compression();
    ~prefix_compression();
};