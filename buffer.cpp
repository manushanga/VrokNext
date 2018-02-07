#include "util/mutil.h"
#include "bufferconfig.h"
#include "buffer.h"

Buffer::Buffer(BufferConfig &config, int id) :
    _id(id),
    _stream_id(0UL)
{
    _config= config;
    _size=config.channels*config.frames;
    _buffer = (real_t *)mutil_aligned_alloc(_size * sizeof(real_t));
    _type = Type::StreamBuffer;
}

Buffer::~Buffer()
{
    mutil_aligned_free(_buffer);
}

void Buffer::Reset(BufferConfig *config)
{
    _config = *config;

    if (_size < _config.frames * _config.channels)
    {
        mutil_aligned_free( _buffer );
        _buffer = (real_t *)mutil_aligned_alloc( _config.frames * _config.channels * sizeof(real_t));
    }
    _size=_config.channels*_config.frames;
}
