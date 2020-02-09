from collections import defaultdict
from collections import deque


class Node:
    def __init__(self, val: set):
        self.val = sorted(val) 
        self.left = None
        self.right = None

    def __lt__(self, other):
        if other == None:
            return False

        if not isinstance(other, Node):
            raise TypeError

        for i in range(len(self.val)):
            if self.val[i][0] < other.val[i][0]:
                return True
            elif self.val[i][0] > other.val[i][0]:
                return False
          
        for i in range(len(self.val)):
            if self.val[i][1] < other.val[i][1]:
                return True
            elif self.val[i][0] > other.val[i][0]:
                return False
        return False

    def __gt__(self, other):
        if other == None:
            return False

        if not isinstance(other, Node):
            raise TypeError

        for i in range(len(self.val)):
            if self.val[i][0] > other.val[i][0]:
                return True
            elif self.val[i][0] < other.val[i][0]:
                return False
          
        for i in range(len(self.val)):
            if self.val[i][1] > other.val[i][1]:
                return True
            elif self.val[i][0] < other.val[i][0]:
                return False
        return False


    def __eq__(self, other):
        if other == None:
            return False

        if not isinstance(other, Node):
            raise TypeError

        for i in range(len(self.val)):
            if self.val[i][0] != other.val[i][0]:
                return False
          
        for i in range(len(self.val)):
            if self.val[i][1] != other.val[i][1]:
                return False
        return True



    def __str__(self):
        return str(self.val)



class BST:
    def __init__(self, root=None):
        self.root = root

    def insert(self, alignment):
       
        comb = Node(alignment)
        if self.root == None:
            self.root = comb
            return True
            
        curr = self.root 
        while True:
            if comb < curr:
                if curr.left:
                    curr = curr.left
                else:
                    curr.left = comb
                    return True
            
            elif comb > curr:
                if curr.right:
                    curr = curr.right
                else:
                    curr.right = comb
                    return True
            else:
                return False


    def print_bst(self):
        q = deque()
        q.append(self.root)
        while len(q) > 0:
            for i in range(len(q)):
                n = q.popleft()
                print(n, end = " ")
                if n is None:
                    continue
                q.append(n.left)
                q.append(n.right)
            print()

            


class alignCombinations:
    def __init__(self):
        self.buckets = defaultdict(BST)
        
    def insertalignment(self, k:int, alignment:set):      
        return self.buckets[k].insert(alignment)

    def print_buckets(self):
        for k,v in self.buckets.items():    
            print(k,":",v)



