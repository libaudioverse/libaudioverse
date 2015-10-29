"""Write a file, if and only if the new contents haven't changed."""
import os.path
import hashlib

def maybe_write(path, contents):
    rewrite = True
    if os.path.exists(path):
        with open(path, 'rb') as f:
            hash1 = hashlib.sha256()
            hash2 = hashlib.sha256()
            hash1.update(contents.encode('utf8'))
            hash2.update(f.read())
            rewrite = hash1.digest() != hash2.digest()
    if rewrite:
        with open(path, 'wb') as f:
            f.write(contents.encode('utf8'))
