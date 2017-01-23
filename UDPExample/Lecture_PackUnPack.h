// PACKING

// pack a byte
#define packByte(bf,b)\
	*bf++ = b;

// Pack a short (16-bit)
#define packShort(bf, s)\
	*bf++ = (s>>8)& 0xFF; \
	*bf++ = (s)& 0xFF;

// Pack a long (32-bit)
#define packLong(bf, l)\
	*bf++ = (l>>24)& 0xFF; \
	*bf++ = (l>>16)& 0xFF; \
	*bf++ = (l>>8)& 0xFF; \
	*bf++ = (l)& 0xFF;

// Pack a number of bytes
#define packBytes(bf, bytes, len)\
	memcpy(bf, bytes, len);\
	bf += len;

// UNPACKING
// Unpack a byte
#define unpackByte(bf, b)\
	b = *bf++;

// Delete
	// Pack a short (16-bit)
	#define packShort(bf, s)\
		*bf++ = (s>>8)& 0xFF; \
		*bf++ = (s)& 0xFF;

	// Pack a long (32-bit)
	#define packLong(bf, l)\
		*bf++ = (l>>24)& 0xFF; \
		*bf++ = (l>>16)& 0xFF; \
		*bf++ = (l>>8)& 0xFF; \
		*bf++ = (l)& 0xFF;
// Delete


// Unpack a short (16-bit)
#define unpackShort(bf, s)\
	s = (*bf++<<8); \
	s = *bf++;

// Unpack a long (32-bit)
#define unpackLong(bf, l)\
	l = (*bf++<<8); \
	l = (*bf++<<8); \
	l = (*bf++<<8); \
	l = *bf++;

// Unpack a number of bytes
#define unpackBytes(bf, bytes, len) \
	memcpy(bytes, bf, len); \
	bf += len;