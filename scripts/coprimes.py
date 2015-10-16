import sys

def euclid(a, b):
    if b > a:
        a, b= b, a
    while b !=0:
        t=b
        b =a%b
        a=t
    return a

if len(sys.argv) != 3:
    print("Usage: coprimes.py <start> <count>")

print("Generating sequence")
start= int(sys.argv[1])
count=int(sys.argv[2])
coprimes=set([start])
while len(coprimes) < count:
    j = max(coprimes)+1
    for i in coprimes:
        while euclid(i, j)!=1:
            j+=1
    coprimes.add(j)
result=sorted(coprimes)

print(", ".join((str(i) for i in result)))