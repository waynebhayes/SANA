import numpy as np

class EmptyList(Exception):
    pass

class my_queue():
    """
    a priority queue in the form of a sorted list
    this can be done with linear insertion,
    but the bisect library might provide a better runtime
    """
    def __init__(self):
        """
        creates a list to store the entries
        each entry must be in the form:
        [val, pairs]
        mutable tuples might be advantageous
        """
        self.pq = [] 
        self.key_fn = lambda tup : tup[0] 

    def __getitem__(self, index):
        """
        just in case external functions need access
        """
        return self.pq[index]

    def insert(self, item):
        self.pq.append(item)
        self.pq.sort(key = self.key_fn, reverse=True)

    def enqueue(self, lst):
        """
        add a whole list of values at once. This saves on sorting
        as the queue does not need to be sorted after each insert. 
        """
        self.pq.entend(lst)
        self.pq.sort(key = self.key_fn, reverse=True)

    def requeue(self, val, lst):
        """
        place a popped item back onto the stack.
        It should still have the highest similarity value.
        Therefore, no sort needed.
        """
        self.pq.insert(0, (val, lst))

    def pop_equals(self):
        """
        rewrite this to be more efficient, now that pq is sorted
        throws IndexError if pq is empty or nothing found.
        """
        val = self.key_fn(self.pq[0])        
        i = 0
        n = len(self.pq)
        while i < n and self.key_fn(self.pq[i]) == val :
            i += 1
        equals = [arr for (value, arr) in self.pq[:i]]
        self.pq = self.pq[i:]
        return (val, np.vstack(equals))
        
    def __repr__(self):
        if not self.pq:
            return "[]"
        return "\n".join([str(x) for x in self.pq])
    
if __name__ == "__main__":
    foo = my_queue()
    foo.insert((1, [1,2,3]))
    (val, lst) = foo.pop_equals()
    print("foo", foo)
    print(val,lst)
    
    bar = my_queue()
    bar.insert((1, [[1,2]]))
    bar.insert((1, [[2,3],[3,4]]))
    bar.insert((0.87, [[5,6],[1,4]]))
    bar.insert((0.86, [[2,4],[2,8]]))
    bar.insert((0.90, [[5,4]]))
    bar.insert((0.23, [[16,32],[32,64]]))
    bar.insert((1, [[3,5],[7,11]]))
    print("bar", bar)
    (val, lst) = bar.pop_equals()
    print("bar", bar)
    print(val,lst)

    baz = my_queue()


    
