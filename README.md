# Expand Int
Expandable integer for c++ 


This repository realizate expandable unsigned, signed and *real* int with fixed bits count using templates.

For example `expand_int<0>` will be equal 128 bit size, `expand_int<1>` is 256 bits, `expand_int<2>` is 512 bits, e.t.c.


# Usage

just use `expand_int<0>` or `int128_ext` as value type

if you wanna unsigned int, add `u` to name near int `expand_uint<0>` & `uint128_ext`

# 

This project use [this](http://github.com/calccrypto/uint256_t/ "calccrypto uint256_t repo") repo as base

# Archive
This repo is archived, instead this use [large_math](http://github.com/GParcade/large_math/ "GParcade large_math repo") repo 
