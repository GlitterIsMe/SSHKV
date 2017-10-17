//
// Created by zzyyy on 2017/4/24.
//

#ifndef SSHKV_TESTUTIL_H
#define SSHKV_TESTUTIL_H

#include "slice.h"
#include "Random.h"

namespace sshkv{

	Slice RandomString(Random* rnd, int len, std::string* dst) {
		dst->resize(len);
		for (int i = 0; i < len; i++) {
			(*dst)[i] = static_cast<char>(' ' + rnd->Uniform(95));   // ' ' .. '~'
		}
		return Slice(*dst);
	}

	std::string RandomKey(Random* rnd, int len) {
		// Make sure to generate a wide variety of characters so we
		// test the boundary conditions for short-key optimizations.
		static const char kTestChars[] = {
				'\0', '\1', 'a', 'b', 'c', 'd', 'e', '\xfd', '\xfe', '\xff'
		};
		std::string result;
		for (int i = 0; i < len; i++) {
			result += kTestChars[rnd->Uniform(sizeof(kTestChars))];
		}
		return result;
	}


	extern Slice CompressibleString(Random* rnd, size_t len, std::string* dst) {
		int raw = static_cast<int>(len);
		if (raw < 1) raw = 1;
		std::string raw_data;
		RandomString(rnd, raw, &raw_data);

		// Duplicate the random data until we have filled "len" bytes
		dst->clear();
		while (dst->size() < len) {
			dst->append(raw_data);
		}
		dst->resize(len);
		return Slice(*dst);
	}

}
#endif //SSHKV_TESTUTIL_H
