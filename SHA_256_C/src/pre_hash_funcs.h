/*
 ============================================================================
 Name        : pre_hash_funcs.h
 Author      : NEOlson
 Version     : 1
 Copyright   : N/A
 Date        : Feb 24, 2022
 Description :

 ============================================================================
 */


#ifndef PRE_HASH_FUNCS_H_
#define PRE_HASH_FUNCS_H_

#include <stdint.h>

uint32_t* pad_msg(uint64_t words_of_padding, uint64_t msg_len_words);


#endif /* PRE_HASH_FUNCS_H_ */