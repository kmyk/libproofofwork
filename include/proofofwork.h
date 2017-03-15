enum { pow_md5_chunk_length = 64 };
enum { pow_md5_digest_length = 16 };
/**
 * @param [in]     mask       the length must be pow_md5_digest_length.
 * @param [in]     target     the length must be pow_md5_digest_length.
 * @param [in,out] buffer     the length must be pow_md5_digest_length.
 * @param [in,out] size       input: must be <= 44, buffer[0, size) is used as a prefix of texts.
 * @return                    whether a text is found or not.
 */
bool pow_md5_mine(uint8_t *mask, uint8_t *target, uint8_t *buffer, uint64_t *size);
