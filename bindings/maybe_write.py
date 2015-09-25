"""Write a file, if and only if the new contents haven't changed."""
import os.path
import hashlib

def maybe_write(path, contents):
    rewrite = True
    if os.path.exists(path):
        with file(path, 'rb') as f:
            hash1 = hashlib.sha256()
            hash2 = hashlib.sha256()
            hash1.update(contents)
            hash2.update(f.read())
            rewrite = hash1.digest() == hash2.digest()
    if rewrite:
        with file(path, 'wb') as f:
            f.write(contents)
