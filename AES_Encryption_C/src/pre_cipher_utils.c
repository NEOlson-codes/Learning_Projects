/*
 ============================================================================
 Name        : pre_cipher_utils.c
 Author      : NEOlson
 Version     : 1
 Copyright   : N/A
 Date        : Feb 21, 2022
 Description : Contains several functions for defining global variables and
               performing other work that is set-up for the cipher process.
               This includes creating the key schedule and defining cipher
               key length-dependent parameters.
 ============================================================================
 */

#include "pre_cipher_utils.h"

// Global declaration of parameter values. Default cipher key is 256 bits.
// Parameterizing some of the function inputs for encryption
// and decryption allows the module to operate with variable
// cipher key lengths.
uint8_t Nb = 4, Nr = 14, Nk = 8;

// Nb is defined as the number of 4-byte words
// in the input data block. AES only takes 128-bit data blocks,
// therefore Nb will always be 4.

// Nk is the number of 4-byte words in the encryption key.
// Nr is the number of rounds of transformations to perform on the state matrix.


// See description in pre_cipher_utils.h
void set_global_params(uint32_t cipher_key_len){

	// Set parameters related to the cipher key length based
	if(cipher_key_len == 256){} // Do nothing. 256-bit cipher key is default
	else if(cipher_key_len == 192){
		Nk = 6;
		Nr = 12;
	}
	else if(cipher_key_len == 128){
		Nk = 4;
		Nr = 10;
	}
	else abort();
	// If the cipher key length is not valid, abort the program. This
	// will force a failure that causes the the programmer using the
	// module to inspect how they're using it.
}


// See description in pre_cipher_utils.h
uint8_t* generate_key_schedule(uint8_t* cipher_key){

	// 4 words from the schedule key are XOR'ed with the state matrix
	// once at the beginning of the cipher process and during each of the
	// 14 rounds.
	size_t key_schedule_len = 4 * Nb * (Nr + 1);

	// All data will be stored as bytes. Key schedule generator will use word
	// count to keep track of which transform to apply. key_schedule array
    static uint8_t key_schedule[240] = {0};

    // Note: key_schedule could not be "static" and be given its length parametrically.
    // My solution is to give key_schedule the length required by AES-256 and pad
    // the end with zeros if the actual implementation is AES-128 or AES-192.

    size_t word = 0;

    // The Nk words (32 bytes per word) of the cipher key are the first Nk words
	// of the schedule key.
    while(word < Nk){
		key_schedule[word*4] = cipher_key[word*4];
		key_schedule[word*4 + 1] = cipher_key[word*4 + 1];
		key_schedule[word*4 + 2] = cipher_key[word*4 + 2];
		key_schedule[word*4 + 3] = cipher_key[word*4 + 3];
		word++;
	}

	// Each key_schedule value after Nk is a transformation of the
	// previous word in key_schedule. In the following loop,
    // temp_word will already be key_schedule[word - 1], so there is
    // no need for re-assignment. temp_word just has to start off at
    // the correct initial value, which is key_schedule[(Nk - 1)*4]
    uint8_t temp_word[4] = {
    	key_schedule[(Nk-1)*4],
		key_schedule[(Nk-1)*4 + 1],
		key_schedule[(Nk-1)*4 + 2],
		key_schedule[(Nk-1)*4 + 3]
    };


    uint8_t round_constant = 0;

    // Starting at word Nk, the algorithm for generating the key schedule
    // changes. Depending on the word number, different transformations
    // may be applied to the input bytes.
	while(word < (key_schedule_len / 4)){

		// Every Nk'th word has a special operation
		if(word % Nk == 0){
			// The word has following transformation:
			// [b0, b1, b2, b3] -> [b1, b2, b3, b0]
			// Note: transformations are in-line for efficiency
			uint8_t temp_byte = temp_word[0];
			temp_word[0] = temp_word[1];
			temp_word[1] = temp_word[2];
			temp_word[2] = temp_word[3];
			temp_word[3] = temp_byte;

			// Now apply the s-box to every individual byte
			for(uint8_t i = 0; i < 4; i++) temp_word[i] = apply_sbox(temp_word[i]);

			// Compute the round constant array in order to XOR it with temp
			uint8_t modifier = word / Nk;
			if(modifier == 1) round_constant = 1;
			else round_constant = mult_by_x(1 , modifier - 1); // First round constant is x^0

			// XOR temp and round constant. Bytes 2, 3, and 4 of round constant are 0.
			temp_word[0] ^= round_constant;
		}
		// One extra operation for 256-bit AES
		else if(Nk > 6 && (word % Nk == 4)){
			// Apply the s-box to every individual byte
			for(uint8_t i = 0; i < 4; i++) temp_word[i] = apply_sbox(temp_word[i]);
		}

		// Perform final XOR (with word Nk words back) to get key_schedule(word)
		temp_word[0] ^= key_schedule[(word - Nk)*4];
		temp_word[1] ^= key_schedule[(word - Nk)*4 + 1];
		temp_word[2] ^= key_schedule[(word - Nk)*4 + 2];
		temp_word[3] ^= key_schedule[(word - Nk)*4 + 3];

		key_schedule[word*4] = temp_word[0];
		key_schedule[(word*4) + 1] = temp_word[1];
		key_schedule[(word*4) + 2] = temp_word[2];
		key_schedule[(word*4) + 3] = temp_word[3];

		word++;
	}

	// Return the pointer to the first element of the key_schedule array
	return key_schedule;
}


uint8_t mult_by_x(uint8_t byte, uint8_t num_multiplications){

	// Check if the 7th bit is a one. If yes then reduce the polynomial
	for(uint8_t i = 1; i <= num_multiplications; i++){
		// Need to check if 8th bit of input byte is 1. If yes, xor is required.
		if(byte > 127){
			byte <<= 1;
			byte ^= 0x1b;
		}
		else byte <<= 1;
	}

	return byte;
}

