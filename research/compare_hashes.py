#!/usr/bin/env python

#
# Experiment to compare various basic hash functions.
#
# Peter H. Froehlich <phf at acm dot org>

# I wrote this to play with hash functions since DASM's hash1
# looked suspicious (and it was also duplicated in the code?).
# It turns out that it's not that bad for hashing our opcodes,
# but Dan Bernstein's hash does twice as well for no real cost
# increase, so I'll probably use Dan's instead of Matt's.
#
# Note that perfect hashing is tempting since we know the keys
# in advance, they don't change dynamically (for opcodes!). So
# I gave it some thought (i.e. wasted some brain cycles) before
# realizing that DASM likes to "mix and match" tables, i.e. it
# produces bigger compound tables from smaller ones based on
# what processor is selected. So if we want to preserve that
# aspect of DASM's design, we can't have perfect hashing. :-/
#
# Just as well, I have a feeling that binary search on "static"
# arrays would do even better, improved locality of reference
# and all that. But I want to keep the hashes, they are a big
# thing in DASM. :-)

import sys

WORDS = []

# We use "raw" hash functions without a compression step; if we
# want to evaluate for a given table size, we compress results
# later.

def hash_dillon(s):
  hash = 0
  for c in s:
    hash = (hash << 2) ^ ord(c)
  return hash

def hash_djb1(s):
  hash = 5381
  for c in s:
    hash = ((hash << 5) + hash) + ord(c) # hash * 33 + c
  return hash

def hash_djb2(s):
  hash = 5381
  for c in s:
    hash = ((hash << 5) + hash) ^ ord(c)
  return hash

def hash_sdbm(s):
  hash = 0
  for c in s:
    hash = ord(c) + (hash << 6) + (hash << 16) - hash
  return hash

def hash_silly(s):
  hash = 0
  for c in s:
    hash = hash + ord(c)
  return hash

def hash_silly2(s):
  hash = 0
  i = 1
  for c in s:
    hash = hash + i*ord(c)
    i += 1
  return hash

def hash_silly3(s):
  hash = 0
  i = 1
  for c in s:
    # these work pretty well for some reason (for opcodes anyway)
    #hash = hash + i**1*ord(c)**3
    hash = hash + i**2*ord(c)**3
    i += 1
  return hash

def hash_knuth(s):
  hash = len(s)
  for c in s:
    hash = ((hash<<5)^(hash>>27))^ord(c)
  return hash

# http://smallcode.weblogs.us/2008/01/22/hash-functions-an-empirical-comparison/
def hash_kankowski(s):
  hash = 0
  for c in s:
    hash = 17 * hash + (ord(c) - ord(' '));
  return hash ^ (hash >> 16);

def eval_hash(hash_func):
  d = {}
  for w in WORDS:
    h = hash_func(w)
    d[h] = d.get(h, 0) + 1
  return d

# Note that DASM uses & for compression; % is the same for positive
# numbers (*unsigned* anyway in DASM), but it could cost a few more
# cycles.

def compress_dict(d, size):
  c = {}
  for k in d:
    n = k % size
    c[n] = c.get(n, 0) + d[k]
  return c

def print_eval(d,s):
  print "------%s-----" % (s)
  print "Unique hash values: %s" % (len(d))
  print "Max collisions: %s" % (max(d.values()))
  print "Min collisions: %s" % (min(d.values()))
  print "Avg collisions: %s" % (sum(d.values())/float(len(d)))
  print "Med collisions: %s" % (sorted(d.values())[len(d)/2])

# read file into WORDS list, assumes one word per line
f = open(sys.argv[1])
for l in f:
  l = l.strip()
  WORDS.append(l)
f.close()

FUNCS = [hash_dillon, hash_djb1, hash_djb2, hash_sdbm,
         hash_silly, hash_silly2, hash_silly3,
         hash_knuth, hash_kankowski]

# TODO: generate some nice graphics with PyX?

for f in FUNCS:
  print "========== %s ==========" % f.__name__
  # just the hash code please
  d = eval_hash(f)
  print_eval(d,"raw")
  # Matt's 2.16 DASM uses 4096 for symbols, 1024 for mnemonics
  c = compress_dict(d, 4096)
  print_eval(c,"4096")
  # our DASM uses 1024 for both hash tables right now...
  c = compress_dict(d, 1024)
  print_eval(c,"1024")

