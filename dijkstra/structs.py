from collections import defaultdict
import heapq as hq

class alignCombinations:
    def __init__(self):
        self.buckets = defaultdict(list)
        
    def insertalignment(self, k:int, alignment:set):      
        if k in self.buckets and alignment in self.buckets[k]:
            return False

        hq.heappush(self.buckets[k], alignment)
        return True

    def print_buckets(self):
        for k,v in self.buckets.items():
            print(k,":",v)
